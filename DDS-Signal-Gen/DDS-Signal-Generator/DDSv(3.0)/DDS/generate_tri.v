module generate_tri (
    input [24:0]f,
    input rst,
    input clk,
    output reg [13:0]triangle=0
);
   
    wire [31:0]phase;
    reg[13:0]pre_triangle=0;
    always@(posedge clk or negedge rst)
        if(!rst)begin 
            pre_triangle<=0;
            triangle<=0;
        end
        else begin
            pre_triangle<=phase[31]?phase[30:17]:~phase[30:17];
            triangle<=pre_triangle;
        end
    phase P(
		.f(f),
		.rst(rst),
		.clk(clk),
		.phase(phase)
	);
endmodule
