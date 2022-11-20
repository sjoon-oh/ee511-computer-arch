/****************************************
    CACHE_CONTROLLER.v

    EE511 2022 CACHE
    Project 3
    
****************************************/


/////////////////////////////
//  TOP MODULE OF CACHE
/////////////////////////////

module CACHE_CONTROLLER 
#(
  parameter INIT_FTRAM0_FILE = "init_ftram0.hex",
  parameter INIT_FTRAM1_FILE = "init_ftram1.hex",
  parameter INIT_DRAM0_FILE = "init_dram0.hex",
  parameter INIT_DRAM1_FILE = "init_dram1.hex"
)
(
    input     wire              CLK,
    input     wire              RST,
    input     wire    [31:0]    DATA_IN_CPU,
    output    wire    [31:0]    DATA_OUT_CPU,
    input     wire    [31:0]    DATA_IN_MEM,
    output    wire    [31:0]    DATA_OUT_MEM,
    input     wire    [31:0]    ADDR_CPU,
    output    wire    [31:0]    ADDR_MEM,
    input     wire              REQ_CPU,
    output    wire              REQ_MEM,
    input     wire              nRW_CPU,
    output    wire              nRW_MEM,
    output    wire              STALL_CPU,
    input     wire              VALID_MEM
);

localparam DWIDTH = 32;
localparam INDEXSIZE = 7;
localparam LINENUM = 128;
localparam BLOCKNUM = 4;
localparam FLAGSIZE = 3;
localparam TAGSIZE = 21;
localparam FLAGTAGSIZE = FLAGSIZE + TAGSIZE;
localparam CACHELINESIZE = DWIDTH * BLOCKNUM;

/********************************
  Write your own code here.
*********************************/

    /* My integration : EE511 : Project 3, 2-way Set-associative Cached Design
     * Author : Sukjoon Oh, sjoon@kaist.ac.kr
     */

    wire [FLAGTAGSIZE - 1:0]      data_from_tagram_0 ,
                                  data_from_tagram_1 ;

    wire [CACHELINESIZE - 1:0]    data_from_dataram_0 ,
                                  data_from_dataram_1 ;


    wire                          wen_tagram_0 ,
                                  wen_tagram_1 ,
                                  wen_dataram_0 ,
                                  wen_dataram_1 ;

    wire [FLAGTAGSIZE - 1:0]      data_to_tagram_0 ,
                                  data_to_tagram_1 ;
    wire [CACHELINESIZE - 1:0]    data_to_dataram ;

    wire [INDEXSIZE - 1:0]        index ;

    DPOnChipMem
    #(
      .BW(FLAGTAGSIZE),
      .AW(INDEXSIZE),
      .ENTRY(LINENUM),
      .INIT_FILE(INIT_FTRAM0_FILE)
      )
      FLAG_TAG_RAM0 
      (
      .CLK     (CLK),
      .CEN1    (~REQ_CPU),
      .CEN2    (),
      .A1      (index),
      .A2      (),
      .WEN1    (wen_tagram_0),
      .WEN2    (),
      .D1      (data_to_tagram_0),
      .D2      (),
      .Q1      (data_from_tagram_0),
      .Q2      ()
    );

    DPOnChipMem
    #(
      .BW(FLAGTAGSIZE),
      .AW(INDEXSIZE),
      .ENTRY(LINENUM),
      .INIT_FILE(INIT_FTRAM1_FILE)
      )
      FLAG_TAG_RAM1 
      (
      .CLK     (CLK),
      .CEN1    (~REQ_CPU),
      .CEN2    (),
      .A1      (index),
      .A2      (),
      .WEN1    (wen_tagram_1),
      .WEN2    (),
      .D1      (data_to_tagram_1),
      .D2      (),
      .Q1      (data_from_tagram_1),
      .Q2      ()
    );

    OnChipMem
    #(
      .BW(CACHELINESIZE),
      .AW(INDEXSIZE),
      .ENTRY(LINENUM),
      .INIT_FILE(INIT_DRAM0_FILE)
      )
      DATA_RAM0
      (
      .CLK     (CLK),
      .CEN     (~REQ_CPU),
      .A       (index),
      .WEN     (wen_dataram_0),
      .D       (data_to_dataram),
      .Q       (data_from_dataram_0)
    );

    OnChipMem
    #(
      .BW(CACHELINESIZE),
      .AW(INDEXSIZE),
      .ENTRY(LINENUM),
      .INIT_FILE(INIT_DRAM1_FILE)
      )
      DATA_RAM1
      (
      .CLK     (CLK),
      .CEN     (~REQ_CPU),
      .A       (index),
      .WEN     (wen_dataram_1),
      .D       (data_to_dataram),
      .Q       (data_from_dataram_1)
    );

    /* My integration : EE511 : Project 3, 2-way Set-associative Cached Design
     * Author : Sukjoon Oh, sjoon@kaist.ac.kr
     */

     drive_machine_2 state_machine_driver(
      // Inputs
      .CLK                  (CLK),
      .RST                  (RST),
      .REQ_CPU              (REQ_CPU),
      .nRW_CPU              (nRW_CPU),
      .DATA_IN_CPU          (DATA_IN_CPU),
      .DATA_IN_MEM          (DATA_IN_MEM),
      .ADDR_CPU             (ADDR_CPU),
      .VALID_Q              (VALID_MEM),
      .data_from_tagram_0   (data_from_tagram_0),
      .data_from_tagram_1   (data_from_tagram_1),
      .data_from_dataram_0  (data_from_dataram_0),
      .data_from_dataram_1  (data_from_dataram_1),

      // Outputs
      .STALL_CPU            (STALL_CPU),
      .ADDR_MEM             (ADDR_MEM),
      .wen_tagram_0         (wen_tagram_0),
      .wen_tagram_1         (wen_tagram_1),
      .wen_dataram_0        (wen_dataram_0),
      .wen_dataram_1        (wen_dataram_1),
      .data_to_tagram_0     (data_to_tagram_0),
      .data_to_tagram_1     (data_to_tagram_1),
      .data_to_dataram      (data_to_dataram),
      .REQ_MEM              (REQ_MEM),
      .nRW_MEM              (nRW_MEM),
      .DATA_OUT_CPU         (DATA_OUT_CPU),
      .DATA_OUT_MEM         (DATA_OUT_MEM),
      .index                (index)
     );

endmodule


/*
    IF YOU WANT TO TEST YOUR DESIGN,
    PLEASE UTILIZE THE GIVEN TESTBENCH FILE (i.e. testbench.v),
    INSTANTIATE A KRP AND MEMORIES IN THAT FILE,
    AND WIRE THEM APPROPRIATELY.

    THEN, MODIFY THE VERILOG LIST FILE (i.e. test.f),
    INCLUDE model.v, CACHE_CONTROLLER.v, testbench.v, AND OTHER VERILOG FILES YOU CREATED.
*/
