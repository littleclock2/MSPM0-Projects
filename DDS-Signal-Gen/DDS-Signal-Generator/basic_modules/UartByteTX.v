module UartByteTX
       #
       (
           parameter [31: 0]baud_rate = 32'd921600
       )
       (
           input clk_100M,
           input rst_n,

           input [7: 0]data_byte,
           input transmit_en,

           output reg TX,
           output irq
       );

localparam [31: 0] baud_set = (100_000_0000 / baud_rate + 5) / 10;

//wire
wire clk_baud;

wire [31: 0]baud_cnt;

//reg
reg transmit_en_pre;
reg transmit;

reg [9: 0]data_byte_reg;

reg irq_pre;

//发送使能
always@(posedge clk_100M or negedge rst_n)
begin
    if (!rst_n)
    begin
        transmit <= 1'd0;

        transmit_en_pre <= 1'd0;
        irq_pre <= 1'd0;

        data_byte_reg <= 1'd0;
    end
    else
    begin
        transmit_en_pre <= transmit_en;
        irq_pre <= irq;

        if (!transmit_en_pre && transmit_en) //发送使能上升沿
        begin
            transmit <= 1'd1;

            data_byte_reg <= {1'b1, data_byte, 1'b0}; //起始位、停止位
        end
        else
        begin
            if (!irq_pre && irq) //发送完成
            begin
                transmit <= 1'd0;
            end
        end
    end
end

//波特时钟
ClkDiv #(baud_set, baud_set[31: 1])ClkDiv_inst
       (
           .clk(clk_100M) ,               	// input  clk_sig
           .rst_n(rst_n) ,               	// input  rst_n_sig
           .phase_rst(~transmit) ,               	// input  phase_rst_sig
           .clk_div(clk_baud) ,                // output  clk_div_sig
           .cnt() 	// output [31:0] cnt_sig
       );

//码元计数
ClkDiv #(32'd10, 32'd5)ClkDiv_inst2
       (
           .clk(clk_baud) ,               	// input  clk_sig
           .rst_n(rst_n) ,               	// input  rst_n_sig
           .phase_rst(~transmit) ,               	// input  phase_rst_sig
           .clk_div(irq) ,                // output  clk_div_sig
           .cnt(baud_cnt) 	// output [31:0] cnt_sig
       );

always@(negedge clk_baud or negedge rst_n)
begin
    if (!rst_n)
    begin
        TX <= 1'd1;
    end
    else
    begin
        case (baud_cnt) //发送
            0, 1 , 2, 3, 4, 5, 6, 7, 8, 9:
                TX <= data_byte_reg[baud_cnt];
            default:
                TX <= 1'd1;
        endcase
    end
end

endmodule
