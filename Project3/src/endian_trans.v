/*
 * EE511 : Project 2, KRP 2.0 Design in Verilog HDL
 * Author : Sukjoon Oh, sjoon@kaist.ac.kr
 */

// This module converts little endian to big endian.
module endian_transformer (
    input [31:0]        input_32,
    output [31:0]       output_32
);

    assign output_32    = {input_32[7:0], input_32[15:8], input_32[23:16], input_32[31:24]};

endmodule