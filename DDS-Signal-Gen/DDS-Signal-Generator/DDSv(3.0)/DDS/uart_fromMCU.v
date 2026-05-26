module uart_fromMCU 
		#(
           parameter [31: 0]baud_rate = 32'd115200
       	)
		(    
		input RX, 
		input rst,
		input clk,
		output reg irq=0,
		output reg flag=0,
		output reg [47:0]result=0
		);//result 更新两次，尾帧eeee，与ffff    第一次是f_big,第二次是mode,A,f_small,两次分开

	reg [7:0]data;
	reg irq_reg=0;
    wire done;
	reg done_reg=0;

    UartByteRX #(baud_rate)UartByteRX_inst
    (
	    .clk_50M(clk) ,	// input  clk_50M_sig
	    .rst_n(rst) ,	// input  rst_n_sig
	    .RX(RX) ,	// input  RX_sig
	    .data_byte(data) ,	// output [7:0] data_byte_sig
	    .irq(done) 	// output  irq_sig
    );
	 always@(posedge clk or negedge rst)begin
		if(!rst)
			done_reg<=0;
		else
			done_reg<=done;
	 end
	 always @(posedge done_reg or negedge rst) begin
	    if(!rst) begin
			flag<=0;
			irq<=0;
			irq_reg<=0;
            result<=0;
        end
		else if(result[15:0] == 16'heeee&&data==8'hee)begin //遇到第一次帧尾
			flag<=0;
			irq_reg<=1;
		end
		else if(result[15:0] ==16'hffff &&data==8'hff)begin//遇到第二次帧尾
			flag<=1;
			irq_reg<=1;
		end
		else begin
			result<={result[39:0],data};
			irq_reg<=1'b0;
		end
		irq<=irq_reg;
	end
endmodule

