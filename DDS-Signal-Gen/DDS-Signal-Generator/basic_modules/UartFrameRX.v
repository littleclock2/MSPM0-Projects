module UartFrameRX
       #
       (
           parameter [31: 0]baud_rate = 32'd921600
       )
       (
           input clk_100M,
           input rst_n,

           input RX,

           output reg[23: 0]data_frame = 1'd0,
           output reg irq = 1'd0
       );

//data_frame[23:0] 3ff

//wire
wire [7: 0]data_byte;
wire byte_rx_irq;

//reg
reg byte_rx_irq_pre = 1'd0;

reg [39: 0]data_frame_reg = 1'd0;

reg irq_reg = 1'd0;

always@(posedge clk_100M or negedge rst_n)
begin
    if (!rst_n)
    begin
        byte_rx_irq_pre <= 1'd0;

        data_frame_reg <= 1'd0;
        data_frame <= 1'd0;

        irq_reg <= 1'd0;
        irq <= 1'd0;
    end
    else
    begin
        byte_rx_irq_pre <= byte_rx_irq;

        if (!byte_rx_irq_pre && byte_rx_irq) //字节接收中断
        begin
            if (data_frame_reg[15: 0] == 16'hffff && data_byte == 8'hff) //帧尾
            begin
                data_frame <= data_frame_reg[39: 16];
                data_frame_reg <= 1'd0;

                irq_reg <= 1'd1;
            end
            else
            begin
                data_frame_reg <= {data_frame_reg[31: 0], data_byte}; //字节左移

                irq_reg <= 1'd0;
            end
        end

        irq <= irq_reg;
    end
end

//字节接收
UartByteRX #(baud_rate)UartByteRX_inst
           (
               .clk_100M(clk_100M) ,       	// input  clk_100M_sig
               .rst_n(rst_n) ,       	// input  rst_n_sig
               .RX(RX) ,       	// input  RX_sig
               .data_byte(data_byte) ,       	// output [7:0] data_byte_sig
               .irq(byte_rx_irq) 	// output  irq_sig
           );
if (data_frame_reg[15: 0] == 16'hffff && data_byte == 8'hff) //帧尾
begin
    data_frame <= data_frame_reg[39: 16];
    data_frame_reg <= 1'd0;

    irq_reg <= 1'd1;
end
endmodule

