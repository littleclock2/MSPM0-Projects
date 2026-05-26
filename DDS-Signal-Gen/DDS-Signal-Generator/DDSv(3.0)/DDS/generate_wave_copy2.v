
module generate_wave(//这个模块负责产生波型，D是输出出去的波形，别的对照上一个模块
	input rst,
	input [3:0]mode,
	input [23:0]f_big,
	input [15:0]f_small,
	input clk,
	output reg[13:0]D
	);
	wire [15:0]sin_mod,sindata;
	wire [31:0]phase,phase_mod;
	reg [29:0]modulation,modulation_pre;
	reg flag,flag_pre;
	reg [13:0]D_pre;
	//reg cnt=clk_50M/f_big;
	always@(posedge clk or negedge rst)begin
		if(!rst)begin
			modulation<=0;
			D<=0;
			D_pre<=0;
			flag<=0;
			flag_pre<=0;
		end
		else begin
			case(mode)
				4'b0001:begin//正弦波
					D_pre<=sindata[15:2];
					D<=D_pre;
				end
				4'b0010:begin//方波
					D_pre<=phase[30]?0:14'b11_1111_1111_1111;
					D<=D_pre;
				end
				4'b0011:begin//三角波
					D_pre<=phase[31]?~phase[31:17]:phase;
					D<=D_pre;
				end
				
				4'b0101:begin//AM sin
					// sindata_reg<=sindata;
					// modulation<=sindata_reg[14:0]*sin_mod[14:0];
					// modulation_pre<=modulation;
					// flag<=sindata_reg[15]^sin_mod[15];
					// flag_pre<=flag;//存符号
					// D<=flag_pre?14'b10_0000_0000_0000-modulation_pre[29:17]:{1'b1,modulation_pre[29:17]};
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

	Sin U1 (
		.address (phase[31:20]),
		.clock (clk),
		.q (sindata)
	);
	Sin U2 (
		.address (phase_mod[31:20]),
		.clock (clk),
		.q (sin_mod)
	);

endmodule