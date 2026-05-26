module generate_square (
    input [24:0]f,
    input rst,
    input clk,
    output reg [13:0]square=0
);
   
    wire [31:0]phase;
    reg[13:0]pre_square=0;
    always@(posedge clk or negedge rst)
        if(!rst)begin 
            pre_square<=0;
            square<=0;
        end
        else begin
            if(phase[31])begin
                pre_square<=0;
                square<=pre_square;
            end
            else begin
                pre_square<=14'b11_1111_1111_1111;
                square<=pre_square;
            end
        end
    phase P(
		.f(f),
		.rst(rst),
		.clk(clk),
		.phase(phase)
	);
endmodule

 // wire [32: 0] limit = 256_0000_0000 / f;//256*50M/f
    // wire [13:0]phase;
    // reg[13:0]pre_square=0;
    // reg [22:0]cnt=0;
    // always@(posedge clk or negedge rst)begin
    //     if(!rst)
    //         pre_square<=0;
    //     else begin
    //         if(cnt<limit>>1)
    //             cnt<=cnt+128;
    //         else begin
    //             cnt<=0;
    //             pre_square<=~pre_square;
    //             square<=pre_square;
    //         end
    //     end
    // end
