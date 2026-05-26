module SyncChain
       (
           input clk_50M,
           input rst_n,

           input sig,

           output reg sig_sync
       );

//wire

//reg
reg sig_reg;

//锁存两次，防止亚稳态
always@(posedge clk_50M or negedge rst_n)
begin
    if (!rst_n)
    begin
        sig_reg <= 1'd0;
        sig_sync <= 1'd0;
    end
    else
    begin
        sig_reg <= sig;
        sig_sync <= sig_reg;
    end
end

endmodule
