module phase
(
    input [23:0]f,
    input rst,
    input clk,//用50M
    output reg[31:0]phase=0
);
    wire[47:0]temp=f*24'd1125_8999<<1;
    wire [31:0]K_12_20=temp[47:16]; //右移16位
    reg [32:0]phase_pre=0;//多一位，保留进位
    always@(posedge clk or negedge rst)begin
        if(!rst)begin
            phase<=0;
            phase_pre<=0;
        end
        else begin
            if(phase_pre[32])begin
                phase_pre<={1'b0,K_12_20};
                phase<=0;
            end
            else begin
                phase_pre<=phase_pre+K_12_20;
                phase<=phase_pre[31:0];
            end
            
        end
    end

endmodule 