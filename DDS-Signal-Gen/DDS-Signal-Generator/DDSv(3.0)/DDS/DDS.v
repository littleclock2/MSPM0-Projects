module DDS(D,clk,rst,RX,clk_out);
    input clk,rst,RX;
	output  reg [13:0]D; //D是输出的信号，将D接上DAC904模块
	output clk_out;
	wire irq;
	wire[31:0]f_big;
	wire [7:0]A;
	wire [15:0]f_small; 
	wire [3:0]mode;
	wire [47:0]result;
	wire flag;
	wire [13:0]D_pre;
	wire clk_out=clk;


	always@(posedge clk)
		D<=D_pre;


	PLL_100M	PLL_100M_inst (
		.inclk0 ( clk ),
		.c0 ( clk_100M )
		);
	uart_fromMCU uart_fromMCU_inst
	(
		.RX(RX) ,	// input  RX_sig
		.rst(rst) ,	// input  rst_sig
		.clk(clk) ,	// input  clk_sig
		.irq(irq) ,	// output  irq_sig
		.flag(flag) ,	// output  flag_sig
		.result(result) 	// output [47:0] result_sig
	);

		analyze analyze_inst
	(
		.clk(clk),
		.irq_frame(irq) ,	// input  irq_frame_sig
		.rst(rst) ,	// input  rst_sig
		.result(result[47:16]) ,	// input [31:0] result_sig
		.flag(flag) ,	// input  flag_sig
		.mode(mode) ,	// output [3:0] mode_sig
		.f_big(f_big) ,	// output [31:0] f_big_sig
		.f_small(f_small) ,	// output [15:0] f_small_sig
		.A(A) 	// output [7:0] A_sig
	);
		generate_wave generate_wave_inst
	(
		.rst(rst) ,	// input  rst_sig
		.mode(mode) ,	// input [3:0] mode_sig
		.f_big(f_big[23:0]) ,	// input [24:0] f_big_sig	// input  irq_frame_sig
		.f_small(f_small) ,	// input [15:0] f_small_sig
		.clk(clk) ,	// input  clk_sig
		.ma(A),//让
		.D(D_pre) 	// output [13:0] D_sig
	);
endmodule