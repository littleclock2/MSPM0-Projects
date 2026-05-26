//这个模块进行有符号整数的加法
module plus_signed#(
    parameter bit_big=8,
    parameter bit_small=8
)
(
    input ci,
    input signed [bit_big - 1 : 0]num1,
    input signed [bit_small - 1 : 0]num2,
    output cout_plus,
    output cout_minor,
    output[bit_big - 1 : 0]result
);
    wire [bit_big : 0]temp = ci + num1 + num2;
    assign cout_plus = (!temp[bit_big] && temp[bit_big - 1]);//进位
    assign cout_minor = (temp[bit_big] && temp[bit_big - 1]);//借位
    assign result = {temp[bit_big],temp[bit_big - 2:0]};  //结果
endmodule 