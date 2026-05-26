module UartByteRX
       #
       (
           parameter [31: 0]baud_rate = 32'd921600
       )
       (
           input clk_50M,
           input rst_n,

           input RX,

           output reg[7: 0]data_byte,
           output reg irq
       );

localparam [31: 0] baud_set = (50_000_0000 / baud_rate + 5) / 10;

//wire
wire RX_sync;

wire clk_baud;

wire [31: 0]baud_cnt;

//reg
reg RX_sync_pre;

reg receive;

reg irq_reg;
reg irq_pre;

reg [8: 0]data_byte_reg;

//同步链
SyncChain SyncChain_inst
          (
              .clk_50M(clk_50M) ,   // input  clk_50M_sig
              .rst_n(rst_n) ,   // input  rst_n_sig
              .sig(RX) ,   // input  sig_sig
              .sig_sync(RX_sync) 	// output  sig_sync_sig
          );

//接收使能
always@(posedge clk_50M or negedge rst_n)
begin
    if (!rst_n)
    begin
        receive <= 1'd0;

        RX_sync_pre <= 1'd0;
        irq_pre <= 1'd0;
    end
    else
    begin
        RX_sync_pre <= RX_sync;
        irq_pre <= irq;

        if (RX_sync_pre && !RX_sync) //RX下降沿
        begin
            receive <= 1'd1;
        end
        else
        begin
            if (!irq_pre && irq) //接受完成
            begin
                receive <= 1'd0;
            end
            else
            begin
                if (baud_cnt == 32'd1 && data_byte_reg[0] != 1'd0) //起始位
                begin
                    receive <= 1'd0;
                end
            end
        end
    end
end

//波特时钟
ClkDiv #(baud_set, baud_set[31: 1])ClkDiv_inst
       (
           .clk(clk_50M) ,      // input  clk_sig
           .rst_n(rst_n) ,       // input  rst_n_sig
           .phase_rst(~receive) ,       // input  phase_rst_sig
           .clk_div(clk_baud) ,         // output  clk_div_sig
           .cnt() 	// output [31:0] cnt_sig
       );

//码元计数
ClkDiv #(32'd10, 32'd5)ClkDiv_inst2
       (
           .clk(clk_baud) ,                                    	// input  clk_sig
           .rst_n(rst_n) ,                                    	// input  rst_n_sig
           .phase_rst(~receive) ,                                    	// input  phase_rst_sig
           .clk_div() ,                                     // output  clk_div_sig
           .cnt(baud_cnt) 	// output [31:0] cnt_sig
       );

always@(negedge clk_baud or negedge rst_n)begin
    if (!rst_n)
    begin
        irq_reg <= 1'd0;

        data_byte_reg <= 1'd0;
        data_byte <= 1'd0;
    end
    else
    begin
        case (baud_cnt) //接收
            0, 1, 2, 3, 5, 6, 7, 8:
            begin
                data_byte_reg[baud_cnt] <= RX_sync;
            end
            4:
            begin
                irq_reg <= 1'd0;

                data_byte_reg[baud_cnt] <= RX_sync;
            end
            9:
            begin
                if (data_byte_reg[0] == 1'd0 && RX_sync == 1'd1) //停止位
                begin
                    data_byte <= data_byte_reg[8: 1];

                    irq_reg <= 1'd1;
                end
            end
            default:
                ;
        endcase
    end
end

always@(posedge clk_50M or negedge rst_n)
begin
    if (!rst_n)
    begin
        irq <= 1'd0;
    end
    else
    begin
        irq <= irq_reg;
    end
end

endmodule
