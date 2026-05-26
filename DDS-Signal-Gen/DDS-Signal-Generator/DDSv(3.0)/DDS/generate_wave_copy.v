module generate_wave(//这个模块负责产生波型，D是输出出去的波形，别的对照上一个模块
	input rst,
	input [3:0]mode,
	input [23:0]f_big,
	input [15:0]f_small,
	input clk_100M,
	output reg[13:0]D
	);
	wire [15:0]sin_mod,sindata;
	wire [48:0]temp=f_big*24'd1125_8999;
	wire [40:0]temp_mod=f_small*24'd1125_8999;
	wire [31:0]K_12_20=temp[48:18],K_12_20_mod=temp_mod[40:10]; //为什么不是[48:17]
	reg[31:0]phase,phase_mod;
	reg [29:0]modulation,modulation_reg;
	reg flag,flag_pre;
	reg [15:0]sindata_reg;
	//reg cnt=clk_50M/f_big;
	always@(posedge clk_100M or negedge rst)begin
		if(!rst)begin
			modulation<=0;
			phase_mod<=0;
			phase<=0;
			D<=0;
		end
		else begin
			phase<=phase+K_12_20;
			phase_mod<=phase_mod+K_12_20_mod;
			case(mode)
				4'b0001:begin//正弦波
				sindata_reg<=sindata;
					D<=sindata_reg[15:2];
				end
				4'b0010:begin//方波
					D<=phase[31]!=0?0:14'b11_1111_1111_1111;
				end
				4'b0011:begin//三角波
					D<=phase[31]?(~phase[30:17]):phase[30:17];
				end
				
				4'b0101:begin//AM sin
					sindata_reg<=sindata;
					modulation<=sindata_reg[14:0]*sin_mod[14:0];
					modulation_reg<=modulation;
					flag<=sindata_reg[15]^sin_mod[15];
					flag_pre<=flag;//存符号
					D<=flag_pre?14'b10_0000_0000_0000-modulation_reg[29:17]:{1'b1,modulation_reg[29:17]};
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

	Sin U1 (
		.address (phase[31:20]),
		.clock (clk_100M),
		.q (sindata)
	);
	Sin U2 (
		.address (phase_mod[31:20]),
		.clock (clk_100M),
		.q (sin_mod)
	);

endmodule