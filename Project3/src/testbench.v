/****************************************
    testbench.v

    EE511 2022 CACHE
    Project 3
    
****************************************/

module testbench;

    reg             CLK, RST;

    /* CLOCK Generator */
    parameter   PER = 10.0;
    parameter   HPER = PER/2.0;

    initial CLK <= 1'b1;
    always #(HPER) CLK <= ~CLK;

    reg        [31:0]    DATA_IN_CPU;
    wire    [31:0]    DATA_OUT_CPU;
    wire    [31:0]    DATA_IN_MEM;
    wire    [31:0]    DATA_OUT_MEM;
    reg        [31:0]    ADDR_CPU;
    wire    [31:0]    ADDR_MEM;
    reg                  REQ_CPU;
    wire              REQ_MEM;
    reg                  nRW_CPU;
    wire              nRW_MEM;
    wire              STALL_CPU;
    wire              VALID_MEM;


    CACHE_CONTROLLER    CACHE_CONTROLLER (
    .CLK                (CLK),
    .RST                (RST),
    .DATA_IN_CPU        (DATA_IN_CPU),
    .DATA_OUT_CPU        (DATA_OUT_CPU),
    .DATA_IN_MEM        (DATA_IN_MEM),
    .DATA_OUT_MEM        (DATA_OUT_MEM),
    .ADDR_CPU            (ADDR_CPU),
    .ADDR_MEM            (ADDR_MEM),
    .REQ_CPU            (REQ_CPU),
    .REQ_MEM            (REQ_MEM),
    .nRW_CPU            (nRW_CPU),
    .nRW_MEM            (nRW_MEM),
    .STALL_CPU            (STALL_CPU),
    .VALID_MEM            (VALID_MEM)
    );


    MainMem        MAIN_MEM    (
        .CLK        (CLK),
        .CEN        (~REQ_MEM),
        .A            (ADDR_MEM[15:2]),
        .WEN        (nRW_MEM),
        .D            (DATA_OUT_MEM),
        .VALID_Q    (VALID_MEM),
        .Q            (DATA_IN_MEM)
    );

    // --------------------------------------------
    // Load test vector to main memory
    // --------------------------------------------
    // Caution : Assumption : input file has hex data like below. 
    //             input file : M[0x03]M[0x02]M[0x01]M[0x00]
    //                    M[0x07]M[0x06]M[0x05]M[0x04]
    //                                    ... 
    //           If the first 4 bytes in input file is 1234_5678
    //           then, the loaded value is mem[0x0000] = 0x1234_5678 (LSB)
    defparam testbench.MAIN_MEM.INITIAL = 1;
    defparam testbench.MAIN_MEM.INIT_FILE = "data.hex";


    initial begin
        RST <= 1'b1;
        #(PER)
        RST <= 1'b0;

    /********************************
      Write your own test code here.
    
      Make your own 'cpu operations' using the 
      'DATA_IN_CPU', 
      'ADDR_CPU',
      'REQ_CPU',
      'nRW_CPU'.
    *********************************/

        /* My integration : EE511 : Project 3, 2-way Set-associative Cached Design
         * Author : Sukjoon Oh, sjoon@kaist.ac.kr
         */

`define REQ_WRITE       1'd0
`define REQ_READ        1'd1

`define ON              1'd1
`define OFF             1'd0

        { ADDR_CPU, DATA_IN_CPU }   <= { 32'd0, 32'd0 } ;
        { REQ_CPU, nRW_CPU }        <= { `OFF, `REQ_READ };

        #(PER) 
        
        // Starts from here. ------------------
        
        { ADDR_CPU, DATA_IN_CPU }   <= { 32'd0, 32'd0 } ;
        { REQ_CPU, nRW_CPU }        <= { `ON, `REQ_READ };
        #(PER)
        { REQ_CPU, nRW_CPU }        <= { `OFF, `REQ_READ }; 

        #(3*PER)
        // 
        { ADDR_CPU, DATA_IN_CPU }   <= { 32'd0, 32'd0 } ;
        { REQ_CPU, nRW_CPU }        <= { `ON, `REQ_WRITE };
        #(PER)
        { REQ_CPU, nRW_CPU }        <= { `OFF, `REQ_WRITE };

        #(3*PER)

        #(10000*PER);

        $writememh("out_ftram0.hex", testbench.CACHE_CONTROLLER.FLAG_TAG_RAM0.ram);
        $writememh("out_ftram1.hex", testbench.CACHE_CONTROLLER.FLAG_TAG_RAM1.ram);
        $writememh("out_dram0.hex", testbench.CACHE_CONTROLLER.DATA_RAM0.ram);
        $writememh("out_dram1.hex", testbench.CACHE_CONTROLLER.DATA_RAM1.ram);
        $writememh("out_data.hex", testbench.MAIN_MEM.ram);
        $finish();
    end

    /* For gtkwave */
    initial begin
        $dumpfile("cache.dmp");
        $dumpvars;
    end

endmodule

