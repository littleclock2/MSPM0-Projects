module analyze(
    input irq_frame,
    input rst,
    input [31:0]result,
    input flag,
    input clk,
    output reg[3:0]mode,
    output reg[31:0]f_big,
	output reg [15:0]f_small,
    output reg[7:0]A
    );
    reg flag_pre,irq_frame_pre;

    /**这里需要加一个在clk作用下寄存flag和irq_frame的模块，这里竞争冒险了*/
    always @(posedge clk or negedge rst) begin
	    if(!rst) begin
            flag_pre<=0;
            irq_frame_pre<=0;
            mode<=0;
            A<=0;
            f_big<=0;
			f_small<=0;
        end
		else begin
            flag_pre<=flag;
            irq_frame_pre<=irq_frame;
            if(irq_frame&&!irq_frame_pre)begin
                if(!flag_pre)begin //遇到第一次帧尾
                    f_big<=result[31:0];
                end
                else begin//遇到第二次帧尾
                    mode<=result[27:24];
                    A<=result[23:16];
                    f_small<=result[15:0];
                end
            end
            else
            ;
        end
	end
endmodule