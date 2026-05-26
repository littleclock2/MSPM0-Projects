module ClkDiv
       #
       (
           parameter [31: 0]cnt_max = 32'd1000,
           parameter [31: 0]cnt_threshold = 32'd500
       )
       (
           input clk,
           input rst_n,

           input phase_rst,

           output reg clk_div,
           output reg [31: 0]cnt
       );

//wire

//reg

//计数分频
//fre_out=fre_in/cnt_max
//cnt=0,1,2,...,cnt_max-1
always@(posedge clk or posedge phase_rst or negedge rst_n)
begin
    if (!rst_n)
    begin
        cnt <= 1'd0;
        clk_div <= 1'd1;
    end
    else
    begin
        if (phase_rst) //相位复位
        begin
            cnt <= 1'd0;
            clk_div <= 1'd1;
        end
        else
        begin
            cnt <= (cnt != cnt_max - 1'd1) ? cnt + 1'd1 : 1'd0; //计数
            if (cnt == cnt_max - 1'd1)
            begin
                clk_div <= 1'd1;
            end
            else if (cnt == cnt_threshold - 1'd1)
            begin
                clk_div <= 1'd0;
            end
        end
    end
end

endmodule
