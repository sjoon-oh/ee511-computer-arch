/*
 * EE511 : Project 3, 2-way Set-associative Cached Design
 * Author : Sukjoon Oh, sjoon@kaist.ac.kr
 */

`define BLOCK_OFFSIZE       2
`define BYTE_OFFSIZE        2

// Sig
`define HIGH                1'b1
`define LOW                 1'b0

`define REQ_WRITE           1'd0
`define REQ_READ            1'd1

module drive_machine_2 #(
    parameter DW = 32, 
        AW = 32, 
        BLOCKNO = 4, 
        FLAGSZ = 3,
        TAGSZ = 21,
        IDXSZ = 7,
        BLKOFFSZ = 2,
        BYTEOFFSZ = 2,
        FLAGTAGSZ = 24
    ) (
    /* INPUTS */
	input                           CLK , 
	input                           RST , 

    input   wire                    REQ_CPU ,
    input   wire                    nRW_CPU ,

    input   wire [DW - 1:0]         DATA_IN_CPU ,
    input   wire [DW - 1:0]         DATA_IN_MEM, 
    input   wire [AW - 1:0]         ADDR_CPU ,
    input   wire                    VALID_Q ,

    input   wire [FLAGTAGSZ - 1:0]  data_from_tagram_0 ,
    input   wire [FLAGTAGSZ - 1:0]  data_from_tagram_1 ,
    input   wire [DW * BLOCKNO - 1:0] data_from_dataram_0 ,
    input   wire [DW * BLOCKNO - 1:0] data_from_dataram_1 ,

    /* OUTPUTS */
    output  reg                     STALL_CPU , 
    output  reg [AW - 1:0]          ADDR_MEM ,  

    output  reg                     wen_tagram_0 ,
    output  reg                     wen_tagram_1 ,
    output  reg                     wen_dataram_0 ,
    output  reg                     wen_dataram_1 ,

    output  reg [FLAGTAGSZ - 1:0]   data_to_tagram_0 ,
    output  reg [FLAGTAGSZ - 1:0]   data_to_tagram_1 ,
    output  reg [DW * BLOCKNO - 1:0] data_to_dataram ,

    output  reg                     REQ_MEM ,
    output  reg                     nRW_MEM ,

    output  reg [DW - 1:0]          DATA_OUT_CPU ,
    output  reg [DW - 1:0]          DATA_OUT_MEM ,

    output  wire [IDXSZ - 1:0]      index // Parsed index
);

    localparam
        STATE_READY                 = 4'd0 ,
        STATE_WRITE_REQ             = 4'd1 ,
        STATE_READ_REQ              = 4'd2 ,
        STATE_MEM_READ              = 4'd3 ,
        STATE_MEM_WRITE             = 4'd4 ,
        STATE_CACHE_WRITE           = 4'd5 ,
        STATE_PENDING_STREAM        = 4'd6 ;

    

    reg [AW - 1:0]              r_addr_cpu ;
    reg [DW - 1:0]              r_data_in_cpu ;
    reg	[DW - 1:0]              r_4byte_to_dataram ;
    reg [DW - 1:0]              r_4byte_to_mem ;

    reg [DW * BLOCKNO - 1:0]    r_cacheline_for_read ;
    reg [DW * BLOCKNO - 1:0]    r_cacheline_for_write ;

    wire                        w_cpu_read_req ,
                                w_cpu_write_req ;

    assign w_cpu_read_req       = REQ_CPU & nRW_CPU ;
    assign w_cpu_write_req      = REQ_CPU & ~nRW_CPU ;


    wire [TAGSZ - 1:0]          w_tag ;
    wire [IDXSZ - 1:0]          w_index;
    wire [BLKOFFSZ - 1:0]       w_blockofs;
    wire [BYTEOFFSZ - 1:0]      w_byteofs;

        // 1. Parser
    assign w_tag            = (r_state == STATE_READY) ? 
                                ADDR_CPU[31:11] : r_addr_cpu[31:11] ;

    assign w_index          = (r_state == STATE_READY) ? 
                                ADDR_CPU[10:4] : r_addr_cpu[10:4] ;

    assign w_blockofs       = (r_state == STATE_READY) ? 
                                ADDR_CPU[3:2] : r_addr_cpu[3:2] ;

    assign w_byteofs        = (r_state == STATE_READY) ? 
                                ADDR_CPU[1:0] : r_addr_cpu[1:0] ;

    assign index            = w_index ;


    // State Variables
    reg [4:0]                   r_state ;
    reg [4:0]                   r_acc_state ;

    reg [FLAGTAGSZ - 1:0]       r_data_from_tagram_0 ,
                                r_data_from_tagram_1 ;
    
    reg [DW * BLOCKNO - 1:0]    r_data_from_dataram_0 ,
                                r_data_from_dataram_1 ;

    wire                        w_valid;
    assign w_valid          = r_data_from_tagram_0[23] & r_data_from_tagram_0[23];

    wire                        w_lrubit_0 ,
                                w_lrubit_1 ;
    assign w_lrubit_0       = r_data_from_tagram_0[22] ;
    assign w_lrubit_1       = r_data_from_tagram_0[22] ;

    wire                        w_dirtybit_0 ,
                                w_dirtybit_1 ,
                                w_dirty ;
    assign w_dirtybit_0     = r_data_from_tagram_0[21] ;
    assign w_dirtybit_1     = r_data_from_tagram_0[21] ;
    assign w_dirty          = w_dirtybit_0 | w_dirtybit_1 ;

    wire                        w_hittag_0 ,
                                w_hittag_1 ,
                                w_hit ;

    assign w_hittag_0   = r_data_from_tagram_0[23] &
                                (w_tag == r_data_from_tagram_0[TAGSZ - 1:0]);
    assign w_hittag_1   = r_data_from_tagram_1[23] &
                                (w_tag == r_data_from_tagram_1[TAGSZ - 1:0]);
    assign w_hit        = w_hittag_0 | w_hittag_1;


    wire [DW - 1:0]    w_bytesel_from_dataram_0;
    wire [DW - 1:0]    w_bytesel_from_dataram_1;

    assign w_bytesel_from_dataram_0     =   (w_byteofs == 2'd0) ?   data_from_dataram_0[7:0]    : 
                                            (w_byteofs == 2'd1) ?   data_from_dataram_0[15:8]   : 
                                            (w_byteofs == 2'd2) ?   data_from_dataram_0[23:16]  : 
                                                                    data_from_dataram_0[31:24]  ;

    assign w_bytesel_from_dataram_1     =   (w_byteofs == 2'd0) ?   data_from_dataram_1[7:0]    : 
                                            (w_byteofs == 2'd1) ?   data_from_dataram_1[15:8]   : 
                                            (w_byteofs == 2'd2) ?   data_from_dataram_1[23:16]  : 
                                                                    data_from_dataram_1[31:24] ;

    reg                         r_is_writing_to_memory ;
    reg	                        r_request ;

    // State Machine starts from here.
    always @ (posedge CLK or posedge RST)
    begin
        if (RST == `HIGH)
        begin
            // Internals
            r_state	            <= STATE_READY ;
            r_addr_cpu          <= 'd0 ;
            r_4byte_to_dataram  <= 'd0;

            r_acc_state         <= 4'd0;
            r_cacheline_for_read    <= 'd0 ;
            r_is_writing_to_memory  <= 1'd0;

            {
                r_data_from_dataram_0, 
                r_data_from_dataram_1, 
                r_data_from_tagram_0,
                r_data_from_tagram_1
            }   <= 'd0 ;

            r_request	        <= `REQ_READ; // Meaningless

            // Outputs
            DATA_OUT_CPU        <= 'd0 ;
            STALL_CPU           <= `LOW ;

            {
                data_to_tagram_0, 
                data_to_tagram_1
            }   <= 'd0 ;

            {
                wen_tagram_0,
                wen_tagram_1,
                wen_dataram_0,
                wen_dataram_1
            }   <= { `REQ_READ, `REQ_READ, `REQ_READ, `REQ_READ } ;

            data_to_tagram_0    <= 0 ;
            data_to_tagram_1    <= 0 ;
            data_to_dataram     <= 0 ;

            ADDR_MEM            <= 'd0 ;

            REQ_MEM             <= `LOW ;
            nRW_MEM             <= `REQ_READ ; // Does not matter


            DATA_OUT_MEM            <= 'd0;
            r_cacheline_for_write   <= 'd0;
        end
        else begin
            case (r_state)

                STATE_READY	:	
                begin
                    // Internals
                    r_addr_cpu          <= ADDR_CPU ;
                    r_acc_state         <= 4'd0 ;
                    r_cacheline_for_read    <= 'd0 ;

                    r_is_writing_to_memory  <= `LOW ;

                    // Transition to corresponding request state
                    if (REQ_CPU && (nRW_CPU == `REQ_WRITE)) begin
                        r_state         <= STATE_WRITE_REQ ;
                        r_request       <= `REQ_WRITE ;
                    end
                    else if (REQ_CPU && (nRW_CPU == `REQ_READ)) begin
                        r_state         <= STATE_READ_REQ ;
                        r_request       <= `REQ_READ ;
                    end
                    else    
                        r_state         <= STATE_READY;

                    // Outputs
                    wen_dataram_0	    <= `REQ_READ ;
                    wen_dataram_1       <= `REQ_READ ;
                    wen_tagram_0	    <= `REQ_READ ;
                    wen_tagram_1	    <= `REQ_READ ; 

                    STALL_CPU           <= `LOW ;

                    REQ_MEM             <= `LOW ;
                    nRW_MEM             <= `REQ_READ ; // Does not matter

                    data_to_tagram_0    <= 'd0 ;
                    data_to_tagram_1    <= 'd0 ;
                    data_to_dataram     <= 'd0 ;
                
                    DATA_OUT_CPU        <= 'd0 ;
                    DATA_OUT_MEM        <= 'd0 ;

                    data_to_dataram	    <= 'd0 ;

                end

                STATE_READ_REQ :	
                begin

                    if (w_hit == `LOW) begin
                        
                        // Case when not hit. 
                        r_data_from_tagram_0        <= data_from_tagram_0 ;
                        r_data_from_tagram_1        <= data_from_tagram_1 ;
                        r_data_from_dataram_0       <= data_from_dataram_0 ;
                        r_data_from_dataram_0       <= data_from_dataram_0 ;

                        STALL_CPU                   <= `HIGH ;

                        wen_tagram_0                <= `REQ_READ ;
                        wen_tagram_1                <= `REQ_READ ;
                        wen_dataram_0               <= `REQ_READ ;
                        wen_dataram_1               <= `REQ_READ ;

                        r_state                     <=  (w_valid == `HIGH) ? 
                                                        STATE_MEM_WRITE :
                                                        STATE_MEM_READ ;
                    end

                    else begin
                        r_state                     <= STATE_READY ;

                        wen_tagram_0                <= `REQ_WRITE ; 
                        wen_tagram_1                <= `REQ_WRITE ;
                        wen_dataram_0               <= `REQ_WRITE ;
                        wen_dataram_1               <= `REQ_WRITE ;                        

                        STALL_CPU                   <= `LOW ;

                        if (w_hittag_0) begin
                            data_to_tagram_0    <= (w_lrubit_0 == `HIGH) ?
                                                    r_data_from_tagram_0 :
                                                    { 
                                                        r_data_from_tagram_0[23],
                                                        `HIGH,
                                                        r_data_from_tagram_0[21:0]
                                                    }; 
                            
                            data_to_tagram_1    <= (w_lrubit_1 == `HIGH) ?
                                                    r_data_from_tagram_1 :
                                                    { 
                                                        r_data_from_tagram_1[23],
                                                        `HIGH,
                                                        r_data_from_tagram_1[21:0]
                                                    }; 

                            DATA_OUT_CPU        <= w_bytesel_from_dataram_0 ;
                        end
                        else begin
                            data_to_tagram_0    <= (w_lrubit_0 == `HIGH) ?
                                                    { 
                                                        r_data_from_tagram_0[23],
                                                        `HIGH,
                                                        r_data_from_tagram_0[21:0]
                                                    } :
                                                    r_data_from_tagram_0;
                            
                            data_to_tagram_1    <= (w_lrubit_1 == `HIGH) ?
                                                    { 
                                                        r_data_from_tagram_1[23],
                                                        `HIGH,
                                                        r_data_from_tagram_1[21:0]
                                                    } :
                                                    r_data_from_tagram_1;

                            DATA_OUT_CPU        <= w_bytesel_from_dataram_1 ;
                        end
                    end
                    end

                STATE_WRITE_REQ	:	
                begin
                    if (w_hit) begin

                        r_data_from_tagram_0        <= data_from_tagram_0;
                        r_data_from_tagram_1        <= data_from_tagram_1;
                        r_data_from_dataram_0       <= data_from_dataram_0;
                        r_data_from_dataram_0       <= data_from_dataram_0;

                        STALL_CPU                   <= `HIGH ;

                        if (VALID_Q == `HIGH)
                            r_state                 <= (w_valid == `HIGH) ? 
                                                        STATE_MEM_WRITE :
                                                        STATE_MEM_READ ;
                        else
                            r_state                 <= r_state;
                    end
                    else begin
                        
                        // Not-hit
                        r_state                     <= STATE_READY ;
                        wen_tagram_0                <= `HIGH ;
                        wen_tagram_1                <= `HIGH ;

                        wen_dataram_0               <= (w_lrubit_0 == `HIGH) ? `REQ_WRITE : `REQ_READ ;
                        wen_dataram_1               <= (w_lrubit_0 == `HIGH) ? `REQ_READ : `REQ_WRITE ;

                        STALL_CPU                   <= `HIGH ;

                        if (w_lrubit_0 == `HIGH)
                        begin
                            case (w_byteofs)
                                2'd0: data_to_dataram <= { 
                                                        data_from_dataram_0[DW * BLOCKNO - 1:DW - 1],     
                                                        r_4byte_to_dataram 
                                                    };
                                2'd1: data_to_dataram <= { 
                                                        data_from_dataram_0[DW * BLOCKNO - 1:DW * 2 - 1],    
                                                        r_4byte_to_dataram,  
                                                        data_from_dataram_0[DW - 1:0]
                                                    };
                                2'd2: data_to_dataram <= { 
                                                        data_from_dataram_0[DW * BLOCKNO - 1:DW * 3 - 1],    
                                                        r_4byte_to_dataram,  
                                                        data_from_dataram_0[DW * 2 - 1:0]
                                                    };
                                2'd3: data_to_dataram <= { 
                                                        r_4byte_to_dataram,
                                                        data_from_dataram_0[DW * 3 - 1:0]
                                                    };
                            endcase
                        end
                        else begin
                            case (w_byteofs)
                                2'd0: data_to_dataram <= { 
                                                        data_from_dataram_1[DW * BLOCKNO - 1:DW - 1],     
                                                        r_4byte_to_dataram 
                                                    };
                                2'd1: data_to_dataram <= { 
                                                        data_from_dataram_1[DW * BLOCKNO - 1:DW * 2 - 1],    
                                                        r_4byte_to_dataram,  
                                                        data_from_dataram_1[DW - 1:0]
                                                    };
                                2'd2: data_to_dataram <= { 
                                                        data_from_dataram_1[DW * BLOCKNO - 1:DW * 3 - 1],    
                                                        r_4byte_to_dataram,  
                                                        data_from_dataram_1[DW * 2 - 1:0]
                                                    };
                                2'd3: data_to_dataram <= { 
                                                        r_4byte_to_dataram,
                                                        data_from_dataram_1[DW * 3 - 1:0]
                                                    };
                            endcase       
                        end

                        data_to_tagram_0    <=  (w_lrubit_1 == `HIGH) ? 
                                                {
                                                    r_data_from_tagram_0[23],     // Valid
                                                    `HIGH,                          // LRU
                                                    r_data_from_tagram_0[21:0]    // Rest
                                                } : {
                                                    `HIGH,  // Valid
                                                    `HIGH,  // LRU
                                                    `LOW,   // Dirty
                                                    r_addr_cpu[31:11]
                                                } ;

                        data_to_tagram_1    <=  (w_lrubit_0 == `HIGH) ? 
                                                {
                                                    r_data_from_tagram_1[23],     // Valid
                                                    `HIGH,                          // LRU
                                                    r_data_from_tagram_1[21:0]    // Rest
                                                } : {
                                                    `HIGH,  // Valid
                                                    `HIGH,   // LRU
                                                    `LOW,   // Dirty
                                                    r_addr_cpu[31:11]
                                                } ; 
                    end
                end
                
                STATE_MEM_READ :	
                begin
                    ADDR_MEM                        <= { r_addr_cpu[AW - 1:2], 2'd0 } ;
                    r_is_writing_to_memory          <= `LOW ;

                    if (VALID_Q == `LOW)
                    begin
                        REQ_MEM                     <= `HIGH ;      // Activate read
                        nRW_MEM                     <= `REQ_READ ; 
                        r_state                     <= r_state;     // Wait for the memory op to finish
                    end
                    else begin
                        REQ_MEM                     <= `LOW ;
                        nRW_MEM                     <= `REQ_READ ; // Does not matter
                        r_state                     <= STATE_CACHE_WRITE ;
                    end
                end

                STATE_PENDING_STREAM :	
                begin
                    if (VALID_Q == `HIGH)
                    begin
                        r_state                     <= (r_is_writing_to_memory == `HIGH) ?
                                                        STATE_MEM_READ : 
                                                        STATE_CACHE_WRITE ; // Here waits for receiver

                        REQ_MEM                     <= `LOW ;
                        nRW_MEM                     <= `REQ_READ ; // Does not matter
                    end
                    else begin
                        if (r_request == `REQ_WRITE)
                        begin
                            DATA_OUT_MEM                <= r_cacheline_for_write[31:0];
                            r_cacheline_for_write       <= { 
                                                            32'd0 , 
                                                            r_cacheline_for_write[DW * BLOCKNO - 1:DW]
                                                        };
                        end

                        r_state                         <= r_state ;
                    end                                
                end

                STATE_MEM_WRITE :	
                begin                
                    if (w_lrubit_0 & w_dirtybit_0)
                    begin    

                        ADDR_MEM            <= { 
                                                r_data_from_tagram_0[20:0],
                                                r_addr_cpu[10:2],
                                                2'd0
                                            };

                        r_cacheline_for_write
                                            <= r_data_from_dataram_0 ;
                    end
                    else if (w_lrubit_1 & w_dirtybit_1) begin

                        ADDR_MEM            <= {
                                                r_data_from_tagram_1[20:0],
                                                r_addr_cpu[10:2],
                                                2'd0
                                            } ;

                        r_cacheline_for_write
                                            <= r_data_from_dataram_1 ;
                    end

                    r_state                 <= STATE_PENDING_STREAM ;
                    
                    if (VALID_Q == `LOW)
                    begin
                        REQ_MEM             <= `HIGH ;       // Activate    
                        nRW_MEM             <= `REQ_WRITE ;
                        r_state             <= STATE_PENDING_STREAM; 
                    end
                    else
                    begin
                        REQ_MEM             <= `LOW ;       // Deactivate
                        nRW_MEM             <= `REQ_WRITE ;
                        r_state             <= r_state;
                    end
                end

                STATE_CACHE_WRITE :	
                begin
                    r_is_writing_to_memory      <= `LOW ;
                    
                    if (r_acc_state != 4'b1111)
                    begin
                        r_cacheline_for_read    <= { DATA_IN_MEM, r_cacheline_for_read[DW * BLOCKNO- 1:DW] };
                        r_acc_state             <= { r_acc_state[2:0], `HIGH };
                    end
                    else begin
                        data_to_dataram         <= r_cacheline_for_read ;
                        r_state                 <= STATE_READY ;

                        wen_tagram_0            <= `HIGH ;
                        wen_tagram_1            <= `HIGH ;

                        // If lru, then set it as a target.
                        data_to_tagram_0        <=  (w_lrubit_0 == `HIGH) ?
                                                    { // Target
                                                        r_data_from_tagram_0[23],     // Valid
                                                        `LOW,                           // LRU
                                                        r_data_from_tagram_0[21:0]    // Rest
                                                    } : {
                                                        `HIGH,  // Valid
                                                        `HIGH,  // LRU
                                                        `LOW,   // Dirty
                                                        r_addr_cpu[31:11]
                                                    } ;

                        data_to_tagram_1        <=  (w_lrubit_0 == `HIGH) ? 
                                                    {
                                                        `HIGH,  // Valid
                                                        `HIGH,  // LRU
                                                        `LOW,   // Dirty
                                                        r_addr_cpu[31:11]
                                                    } : {
                                                        r_data_from_tagram_1[23],     // Valid
                                                        `LOW,                           // LRU
                                                        r_data_from_tagram_1[21:0]    // Rest
                                                    } ;

                        { wen_dataram_0, wen_dataram_1 }    <=  (w_lrubit_0 == `HIGH) ? 
                                                                { `REQ_READ, `REQ_WRITE } : { `REQ_READ, `REQ_WRITE } ;

                        r_acc_state             <= 4'd0 ;
                    end
                end
                        
                default :
                begin
                    r_state         <=  STATE_READY ;
                end
            endcase
        end
    end

endmodule
