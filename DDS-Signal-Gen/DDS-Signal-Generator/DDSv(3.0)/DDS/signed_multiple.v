module signed_multiple
#(
    parameter bit_1=14,
    parameter bit_2=14
)
(
input [bit_1-1:0]num1,
input [bit_2-1:0]num2,
input clk,
input rst,
output reg [bit_1+bit_2-2:0]result
);
    wire flag=num1[bit_1-1]^num2[bit_2-1];
    reg [bit_1+bit_2-3:0] temp;
    reg [bit_1+bit_2-2:0]result_pre;
    always@(posedge clk or negedge rst)begin
       if(!rst) begin
            result<=0;
            temp<=0;
       end
       else begin
            temp<=num1[bit_1-2:0]*num2[bit_2-2:0];
            result<=result_pre;
            result_pre<={flag,temp};
       end
    end
endmodule