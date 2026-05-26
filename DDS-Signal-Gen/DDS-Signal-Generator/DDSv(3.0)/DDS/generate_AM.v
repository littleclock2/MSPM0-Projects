module generate_AM(
    input clk,
    input rst,
    input [23:0]f_carrier,
    input [23:0]f_mod,
    input[7:0]ma,//ma是一个32-128的数字，本来是30%-100%，但是为了便于FPGA处理，转化成了128分比的形式，为了得到百分比，右移8位（会有误差）
    output reg[13:0]AM=0
);

    wire [22:0] after_ma;
    wire [26:0] raw_data;
    wire [13:0] carrier_signal,mod_signal,after_plus;
    reg  [13:0] carrier_signal_signed,mod_signal_signed;//将正弦波转化为有符号的数字信号
    reg  [8:0]  ma_reg = 0;
    reg  [13:0] pre_AM = 0;
    //这个寄存器的作用：1：防止时序问题 2：ma=100,转化为128时，这个信息输入不到ma 里面的，得到的是0，所以这个ma帮助将0转化为128，给了他9位位宽
    always@(posedge clk or negedge rst)begin
        if(!rst)begin
            pre_AM<=0;
        end
        else begin
            ma_reg<=ma==0?9'b1_0000_0000:ma;
            AM<=pre_AM;
            carrier_signal_signed<=carrier_signal[13]?  //载波信号转为有符号信号
                                    {1'b0,carrier_signal[12:0]}://正数
                                    {1'b1,~carrier_signal[12:0]};//负数
            mod_signal_signed   <= mod_signal[13]?     //调制波信号转为有符号信号
                                    {1'b0,mod_signal[12:0]}://正数
                                    {1'b1,~mod_signal[12:0]};//负数
            pre_AM              <= raw_data[26]?
                                    13'b1_0000_0000_0000+raw_data[25:13]:
                                    13'b1_0000_0000_0000-raw_data[25:13];
        end
    end
	
    generate_sin sin_carrier(
		.clk(clk),
		.rst(rst),
		.f(f_carrier),
		.signal(carrier_signal)
	);
    generate_sin sin_modulation(
		.clk(clk),
		.rst(rst),
		.f(f_mod),
		.signal(mod_signal)
	);

    signed_multiple #(//有符号乘法 这一个模块做ma*mod_signal,得到after_ma
        .bit_1(4'd10),
        .bit_2(4'd14)
    )
	U1
    (
        .num1(ma_reg),//正数 ma
        .num2(mod_signal_signed),//乘以调制波的信号
        .clk(clk),
        .rst(rst),
        .result(after_ma)   //得到 ma*128*V(V是调制波的电压数字值)
    );
	 
     plus_signed #(//有符号加法，这个模块负责将after_ma+1_0000_0000_0000,得到after_plus
        .bit_big(4'd14),
        .bit_small(4'd14)
     )
     plus_signed_inst
    (
        .ci(0) ,	// input  ci_sig
        .num1({after_ma[22],after_ma[20:8]}),//取after右移八位的数据
            //这里这么取是因为我虽然是给了after_ma 23个位宽，但是由于ma是30-128的数字，最大也就2的8次方大小
            //乘以mod_signal_signed,(这是个带符号的调制波电压的数字值，位宽14)最多就22位宽所以它的第[21]是绝对是0的
            //这样可以保证我乘出来的数字是只有27位宽的
        .num2(14'h1fff) ,	// input [bit_small-1:0] num2_sig
        .cout_plus() ,	// output  cout_plus_sig
        .cout_minor() ,	// output  cout_minor_sig
        .result(after_plus) 	// output [bit_big-1:0] result_sig
    );

    signed_multiple #(//有符号乘法，将两个已经调整好的有符号正弦波相乘，得到rawdata(27位宽)
        .bit_1(4'd14),
        .bit_2(4'd14)
    )
	U2
    (
        .num1(after_plus),
        .num2(carrier_signal_signed),
        .clk(clk),
        .rst(rst),
        .result(raw_data)
    );
endmodule