module generate_sin(
    input clk,
    input rst,
    input [23:0]f,
    output reg[13:0]signal=0
);
    wire [15:0]sindata;
    wire [31:0]phase;
    reg [13:0]signal_pre;
    always@(posedge clk or negedge rst)
        if(!rst)    signal<=0;
        else begin
            signal_pre<=sindata[15:2];
            signal<=signal_pre;
        end

    phase P(
		.f(f),
		.rst(rst),
		.clk(clk),
		.phase(phase)
	);
    Sin U (
		.address (phase[31:20]),
		.clock (clk),
		.q (sindata)
	);
endmodule