
module generate_wave(//这个模块负责产生波型，D是输出出去的波形，别的对照上一个模块
	input rst,
	input [3:0]mode,
	input [23:0]f_big,
	input [15:0]f_small,
	input clk,
	input [7:0]ma,
	output reg[13:0]D
	);


	wire [13:0]sin_mod,sindata,square,triangle,AM;
	wire [31:0]phase,phase_mod;
	reg [29:0]modulation,modulation_pre;
	reg flag,flag_pre;
	reg [13:0]D_pre;
	//reg cnt=clk_50M/f_big;
	always@(posedge clk or negedge rst)begin
		if(!rst)begin
			D<=0;
			D_pre<=0;
			flag<=0;
			flag_pre<=0;
		end
		else begin
			case(mode)
				4'b0001:begin//正弦波
					D_pre<=sindata;
					D<=D_pre;
				end
				4'b0010:begin//方波
					D_pre<=square;
					D<=D_pre;
				end
				4'b0011:begin//三角波
					D_pre<=triangle;
					D<=D_pre;
				end
				
				4'b0101:begin//AM sin
					D_pre<=AM;
					D<=D_pre;
				end
				4'b1010:begin//FM sin
				;
				end
				4'b0111:begin//AM 三角
				;
				end
				4'b1011:begin//FM 三角
				;
				end
				default:;//没选择波形，不输出
			endcase
		end
	end
	
	generate_sin U1(
		.clk(clk),
		.rst(rst),
		.f(f_big),
		.signal(sindata)
	);
	generate_square U2 (
		.f(f_big),
		.rst(rst),
		.clk(clk),
		.square(square)
	);
	generate_tri U3(
		.f(f_big),
		.rst(rst),
		.clk(clk),
		.triangle(triangle)
	);
	generate_AM U4
(
	.clk(clk) ,	// input  clk_sig
	.rst(rst) ,	// input  rst_sig
	.f_carrier(f_big) ,	// input [23:0] f_carrier_sig
	.f_mod(f_small) ,	// input [23:0] f_mod_sig
	.ma(ma) ,	// input [7:0] ma_sig
	.AM(AM) 	// output [13:0] AM_sig
);
	phase p1(
		.f(f_big),
		.rst(rst),
		.clk(clk),
		.phase(phase)
	);
	phase p2(
		.f({8'b0000_0000,f_small}),
		.rst(rst),
		.clk(clk),
		.phase(phase_mod)
	);

endmodule