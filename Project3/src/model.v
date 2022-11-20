module MainMem #(parameter BW = 32, AW = 14, ENTRY = 1024*16, INITIAL = 1, INIT_FILE="main_mem_in.hex") (
    input    wire                CLK,
    input    wire                CEN,       // CHIP ENABLE (ACTIVE LOW)
    input    wire    [AW-1:0]    A,         // ADDRESS
    input    wire                WEN,       // WRITE ENABLE (1: READ/0: WRITE)
    input    wire    [BW-1:0]    D,         // DATA INPUT
    output   wire                VALID_Q,   // VALID DATA
    output   wire    [BW-1:0]    Q          // DATA OUTPUT
);

    parameter    ATIME    = 1;
    parameter    DELAY    = 9;//Do not change this parameter

    reg     [BW-1:0]    ram[0:ENTRY-1];
    reg     [BW-1:0]    outline;
    reg                 valid_outline;

    reg                 CEN_delay[0:DELAY-1];
    reg     [AW-1:0]    A_delay[0:DELAY-1];
    reg                 WEN_delay[0:DELAY-1];
    reg     [BW-1:0]    D_delay[0:DELAY-1];

    genvar i;

    initial begin
	    if(INITIAL>0)
		    $readmemh(INIT_FILE, ram);
    end

    always @ (posedge CLK)
    begin
        CEN_delay[0] <= CEN;
        A_delay[0] <= A;
        WEN_delay[0] <= WEN;
        D_delay[0] <= D;
    end
    
    generate
        for(i = 0; i < DELAY-1; i=i+1) begin : delay_signals
            always @ (posedge CLK)
            begin
                CEN_delay[i+1] <= CEN_delay[i];
                A_delay[i+1] <= A_delay[i];
                WEN_delay[i+1] <= WEN_delay[i];
                D_delay[i+1] <= D_delay[i];
            end
        end
    endgenerate

    always @ (posedge CLK)
    begin
        if (~CEN_delay[DELAY-1]) begin
            if (WEN_delay[DELAY-1]) begin
                outline                 <= ram[A_delay[DELAY-1]];
                valid_outline           <= 1'b1;
            end
            else begin
                ram[A_delay[DELAY-1]]   <= D_delay[DELAY-1];
                outline                 <= D_delay[DELAY-1];
                valid_outline           <= 1'b1;
            end
        end
        else begin
            valid_outline   <= 1'b0;
        end
    end

    assign    #(ATIME)    Q          = outline;
    assign    #(ATIME)    VALID_Q    = valid_outline;

endmodule

module OnChipMem #(parameter BW = 32, AW = 10, ENTRY = 1024, INITIAL = 1, INIT_FILE="cache_mem_in.hex") (
    input    wire                CLK,
    input    wire                CEN,   // CHIP ENABLE (ACTIVE LOW)
    input    wire    [AW-1:0]    A,     // ADDRESS
    input    wire                WEN,   // WRITE ENABLE (1: READ/0: WRITE)
    input    wire    [BW-1:0]    D,     // DATA INPUT
    output   wire    [BW-1:0]    Q      // DATA OUTPUT
);

    parameter    ATIME    = 1;

    reg        [BW-1:0]    ram[0:ENTRY-1];
    reg        [BW-1:0]    outline;

    initial begin
	    if(INITIAL>0)
		    $readmemh(INIT_FILE, ram);
    end

    always @ (posedge CLK)
    begin
        if (~CEN)
        begin
            if (WEN)    outline <= ram[A];
            else begin
                ram[A]          <= D;
                outline         <= D;
            end
        end
    end

    assign    #(ATIME)    Q    = outline;

endmodule

module DPOnChipMem #(parameter BW = 32, AW = 10, ENTRY = 1024, INITIAL = 1, INIT_FILE="cache_mem_in.hex") (
    input    wire                CLK,
    input    wire                CEN1,   // CHIP ENABLE (ACTIVE LOW)
    input    wire                CEN2,   // CHIP ENABLE (ACTIVE LOW)
    input    wire    [AW-1:0]    A1,     // ADDRESS
    input    wire    [AW-1:0]    A2,     // ADDRESS
    input    wire                WEN1,   // WRITE ENABLE (1: READ/0: WRITE)
    input    wire                WEN2,   // WRITE ENABLE (1: READ/0: WRITE)
    input    wire    [BW-1:0]    D1,     // DATA INPUT
    input    wire    [BW-1:0]    D2,     // DATA INPUT
    output   wire    [BW-1:0]    Q1,      // DATA OUTPUT
    output   wire    [BW-1:0]    Q2      // DATA OUTPUT
);

    parameter    ATIME    = 1;

    reg        [BW-1:0]    ram[0:ENTRY-1];
    reg        [BW-1:0]    outline1;
    reg        [BW-1:0]    outline2;

    initial begin
	    if(INITIAL>0)
		    $readmemh(INIT_FILE, ram);
    end

    always @ (posedge CLK)
    begin
        if (~CEN1)
        begin
            if (WEN1)    outline1 <= ram[A1];
            else begin
                ram[A1]          <= D1;
                outline1         <= D1;
            end
        end
    end

    always @ (posedge CLK)
    begin
        if (~CEN2)
        begin
            if (WEN2)    outline2 <= ram[A2];
            else begin
                ram[A2]          <= D2;
                outline2         <= D2;
            end
        end
    end

    assign    #(ATIME)    Q1    = outline1;
    assign    #(ATIME)    Q2    = outline2;

endmodule