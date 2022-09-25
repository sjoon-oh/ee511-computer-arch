#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "krpiss.h"

int main(int argc, char** argv) {
	unsigned int Inst = 0;
	char str[100];
	char in = NULL;int i = 0;

	/*Special-purpose register initialization*/
	PC = 0;
	IE = 0;
	IPC = 0;
	
	/*Program memory initialization*/
	init_mem(argv[1]);

	/*Program start*/
	printf("========================================================================\n");
	printf("     	 	        KRP 2.0 Instruction Set Simulator\n");
	printf("========================================================================\n");
	printf("Request: \n");

	printf(">> ");
	while(1) {
		in = getchar();
		Inst = (pm[PC+3]<<24) | (pm[PC+2]<<16) | (pm[PC+1]<<8) | pm[PC];

		switch(in) {
				case 's'	: /*Process an instruction*/
					process(Inst);
					view_reg(Inst);
					PC = PC+4;
					break;
				case 'r'	: /*Recursively process*/
					while(PC != (BP<<2)) {
						Inst = (pm[PC+3]<<24) | (pm[PC+2]<<16) | (pm[PC+1]<<8) | pm[PC];
						process(Inst);
						PC = PC+4;
					}
					dump_data_mem(argv[2]);
					break;					
				case 'b'	: /*Set break point*/
					printf("break point: ");
					scanf("%s", str);
					BP = atoi(str);
					break;
				case 'd'	: /*Display gpr*/
					view_reg(Inst);
					break;
				case 'q'	: /*Quit*/
					dump_data_mem(argv[2]);
					exit(0);
					break;
		}

		if (in != '\n')
			printf(">> ");
	}
	
	return 0;
}

void view_reg(unsigned int inst) {
	int i, j, index;                                               
	printf("PC :%08X\nIR :%08X\n", PC, inst);
	printf("IE :%01X\nIPC:%08X\n", IE, IPC);
	for (i=0; i<8; i++) {                                          
		for (j=0; j<4; j++) {                                      
			index = 4*i+j;                                         
			printf("reg[%2d]: %d\t", index, reg[index]); 
			}                                                          
		printf("\n");                                              
	}                                                              
}

int init_mem(char *file_name) {                                      
	int inc;
	unsigned char tmp_char; 
	int addr;
	FILE *fp;

	pm = (unsigned char*)malloc(IMEM_SIZE*sizeof(unsigned char));
	dm = (unsigned char*)malloc(DMEM_SIZE*sizeof(unsigned char));

	inc = 0;
	fp = fopen(file_name, "rb");
	if(fp == NULL) {
		printf("Cannot open the file.\n");
		exit(1);
	}

	while(inc < IMEM_SIZE){
		fread(&tmp_char, sizeof(unsigned char), 1, fp);
		if(feof(fp)) break;

		pm[inc] = tmp_char;
		inc++;
	}
	fclose(fp);

	return 0;
}
 
void dump_data_mem(char *file_name) {                                      
	int inc;
	FILE *fp_dump;

	inc = 0;
	fp_dump = fopen(file_name, "wt");
	if(fp_dump == NULL) {
		printf("Cannot open the file.\n");
		exit(1);
	}

	while(inc < DMEM_SIZE){
		fprintf(fp_dump, "0x%08X: %02X %02X %02X %02X\n", inc, dm[inc+3], dm[inc+2], dm[inc+1], dm[inc]);
		inc+=4;
	}
	fclose(fp_dump);
}
 


/* [EE511] Computer Arch. Project 1
 * -- implemenation starts from here.
 *  Refer to the first commit and the final changelog.
 *
 * Author: Sukjoon Oh, sjoon@kaist.ac.kr
 * */

#define ONES            0xFFFFFFFF

#define NBITS           32
#define NBITS_IMM17     17
#define NBITS_RC        5
#define NBITS_RB        5
#define NBITS_RA        5
#define NBITS_OPCODE    5

#define NBITS_COND		3

#define NBITS_I         1
#define NBITS_SHAMT     5
#define NBITS_MODE      2
#define NBITS_IMM10     10

#define NBITS_IMM22     22

// Type 1
#define OFFS_IMM17      0
#define OFFS_RC         12
#define OFFS_RB         (OFFS_IMM17 + NBITS_IMM17)
#define OFFS_RA        	(OFFS_RB + NBITS_RB)
#define OFFS_OPCODE     (OFFS_RA + NBITS_RA)

#define OFFS_COND		0
#define OFFS_IMM22      0

// Type 2
#define OFFS_SHAMT      0
#define OFFS_I          (OFFS_SHAMT + NBITS_SHAMT)
#define OFFS_MODE       (OFFS_SHAMT + NBITS_SHAMT)
#define OFFS_IMM10      (OFFS_MODE + NBITS_MODE)

#define LSMASK_IMM17    (ONES >> (NBITS - NBITS_IMM17))
#define LSMASK_RC       (ONES >> (NBITS - NBITS_RC))
#define LSMASK_RB       (ONES >> (NBITS - NBITS_RB))
#define LSMASK_RA       (ONES >> (NBITS - NBITS_RA))
#define LSMASK_OPCODE   (ONES >> (NBITS - NBITS_OPCODE))
#define LSMASK_COND     (ONES >> (NBITS - NBITS_COND))
#define LSMASK_I        (ONES >> (NBITS - NBITS_I))
#define LSMASK_SHAMT    (ONES >> (NBITS - NBITS_SHAMT))
#define LSMASK_MODE     (ONES >> (NBITS - NBITS_MODE))
#define LSMASK_IMM10    (ONES >> (NBITS - NBITS_IMM10))

#define LSMASK_IMM22    (ONES >> (NBITS - NBITS_IMM22))

/* Shortened substitution
 * */ 
#define LMI17   LSMASK_IMM17
#define LMRC    LSMASK_RC
#define LMRB    LSMASK_RB
#define LMRA    LSMASK_RA
#define LMO     LSMASK_OPCODE
#define LMC     LSMASK_COND
#define LI      LSMASK_I
#define LMS     LSMASK_SHAMT
#define LMM     LSMASK_MODE
#define LMI10   LSMASK_IMM10
#define LMI22   LSMASK_IMM22

#define NBI17   NBITS_IMM17
#define NBRC    NBITS_RC
#define NBRB    NBITS_RB
#define NBRA    NBITS_RA
#define NBO     NBITS_OPCODE
#define NBI     NBITS_I
#define NBS     NBITS_SHAMT
#define NBM     NBITS_MODE
#define NBI10   NBITS_IMM10
#define NBI22   NBITS_IMM22

#define OI17    OFFS_IMM17
#define ORC     OFFS_RC
#define ORB     OFFS_RB
#define ORA     OFFS_RA
#define OO      OFFS_OPCODE
#define OC      OFFS_COND
#define OI      OFFS_I
#define OS      OFFS_SHAMT
#define OM      OFFS_MODE
#define OI10    OFFS_IMM10
#define OI22    OFFS_IMM22

#define __EXT(inst, offs, lsmask) \
    ((inst >> offs) & lsmask) 

unsigned sign_ext2(unsigned tar, unsigned bits) {

#define __SB(t, sz)   (t >> (sz - 1))
    return (__SB(tar, bits)) ? ((ONES << (32 - bits)) | tar) : tar;
}

// Here lies some global substitutions, 
//  do not delete these abbreviations.
#define I2RA_     __EXT(inst, ORA, LMRA)
#define I2RB_     __EXT(inst, ORB, LMRB)
#define I2RC_     __EXT(inst, ORC, LMRC)
#define I2IMM10_  __EXT(inst, OI10, LMI10)
#define I2IMM17_  __EXT(inst, OI17, LMI17)
#define I2IMM22_  __EXT(inst, OI22, LMI22)
#define I2OP_     __EXT(inst, OO, LMO)
#define I2C_      __EXT(inst, OC, LMC)
#define I2OM_     __EXT(inst, OM, LMM)

#define I2I_      __EXT(inst, OI, LI)
#define I2S_      __EXT(inst, OS, LMS)

unsigned switch_mode_typed2(unsigned inst) {
    unsigned shamt = I2S_;
    unsigned se_imm10 = sign_ext2(I2IMM10_, NBI10);
    switch (I2OM_) {
        case 0: return (se_imm10 << shamt);
        case 1: return (se_imm10 >> shamt);
        case 2: return ((se_imm10 & (ONES << 31)) | (se_imm10 >> shamt));
        case 3: return ((se_imm10 << (32 - shamt)) | (se_imm10 >> shamt));
    }
}

// forward declaration
void __nop(unsigned);
void __neg(unsigned);

void __addi1(unsigned inst) { reg[I2RA_] = reg[I2RB_] + sign_ext2(I2IMM17_, NBI17); }
void __addi2(unsigned inst) { reg[I2RA_] = reg[I2RB_] + switch_mode_typed2(inst); }

void __ori1(unsigned inst) { reg[I2RA_] = reg[I2RB_] | sign_ext2(I2IMM17_, NBI17); }
void __ori2(unsigned inst) { reg[I2RA_] = reg[I2RB_] | switch_mode_typed2(inst); }

void __andi1(unsigned inst) { reg[I2RA_] = reg[I2RB_] & sign_ext2(I2IMM17_, NBI17); }
void __andi2(unsigned inst) { reg[I2RA_] = reg[I2RB_] & switch_mode_typed2(inst); }

void __movi1(unsigned inst) { reg[I2RA_] = sign_ext2(I2IMM17_, NBI17); } 
void __movi2(unsigned inst) { reg[I2RA_] = switch_mode_typed2(inst); }

void __add(unsigned inst) { reg[I2RA_] = reg[I2RB_] + reg[I2RC_]; }
void __sub(unsigned inst) { reg[I2RA_] = reg[I2RB_] + ((~reg[I2RC_]) + 1); }

void __not(unsigned inst) { reg[I2RA_] = ~reg[I2RC_]; }
void __neg(unsigned inst) { unsigned lc = reg[I2RC_]; reg[I2RA_] = (~lc) + 1; }

void __or(unsigned inst)  { reg[I2RA_] = reg[I2RB_] | reg[I2RC_]; }
void __and(unsigned inst) { reg[I2RA_] = reg[I2RB_] & reg[I2RC_]; }
void __xor(unsigned inst) { reg[I2RA_] = reg[I2RB_] ^ reg[I2RC_]; }
void __asr(unsigned inst) {
    reg[I2RA_] = (I2I_) ? 
        ((reg[I2RB_] & (ONES << 31)) | (reg[I2RB_] >> I2S_)) : 
        ((reg[I2RB_] & (ONES << 31)) | (reg[I2RB_] >> reg[I2RC_ & 0b11111]));
}

void __lsr(unsigned inst) { reg[I2RA_] = (I2I_ == 0) ? reg[I2RB_] >> I2S_ : reg[I2RB_] >> (reg[(I2RC_ & 0b11111)]); }
void __shl(unsigned inst) { reg[I2RA_] = (I2I_ == 0) ? reg[I2RB_] << I2S_ : reg[I2RB_] << (reg[(I2RC_ & 0b11111)]); }
void __ror(unsigned inst) {
    reg[I2RA_] = (I2I_) ? 
        ((reg[I2RB_] << (32 - I2S_)) | (reg[I2RB_] >> I2S_)) : 
        ((reg[I2RB_] << (32 - reg[I2RC_ & 0b11111])) | (reg[I2RB_] >> reg[I2RC_ & 0b11111]));
}

void __br(unsigned inst)  {
    switch (I2C_) {
        case 1: PC = reg[I2RB_]; break;
        case 2: if (reg[I2RC_] == 0) PC = reg[I2RB_]; break;
        case 3: if (reg[I2RC_] != 0) PC = reg[I2RB_]; break;
        case 4: if ((reg[I2RC_] & (ONES << 31)) == 0) PC = reg[I2RB_]; break;
        case 5: if ((reg[I2RC_] & (ONES << 31)) != 0) PC = reg[I2RB_]; break;
        default: __nop(inst);
    }
}
void __brl(unsigned inst) { reg[I2RA_] = PC; __br(inst); }

void __j(unsigned inst)   { PC = PC + sign_ext2(I2IMM22_, NBI22); }
void __jl(unsigned inst)  { reg[I2RA_] = PC; PC = PC + sign_ext2(I2IMM22_, NBI22); }

void __ld(unsigned inst)  {
    unsigned ra = I2RA_;
    unsigned maddr = sign_ext2(I2IMM17_, NBI17);

    reg[ra] = (I2RB_ == 0b11111) ? dm[maddr] : dm[maddr + I2RB_];
}
void __ldr(unsigned inst) { reg[I2RA_] = dm[PC + sign_ext2(I2IMM22_, NBI22)]; }
void __st(unsigned inst)  {
    unsigned ra = I2RA_;
    unsigned maddr = sign_ext2(I2IMM17_, NBI17);

    if (I2RB_ == 0b11111) dm[sign_ext2(I2IMM17_, NBI17)] = reg[ra];
    else dm[sign_ext2(I2IMM17_, NBI17) + I2RB_] = reg[ra];
}
void __str(unsigned inst) { dm[PC + sign_ext2(I2IMM22_, NBI22)] = reg[I2RA_]; }

void __lea(unsigned inst) { reg[I2RA_] = PC + sign_ext2(I2IMM22_, NBI22); }
void __nop(unsigned inst) { } 					// OK.
void __ien(unsigned inst) { IE = 1; } 			// OK. 
void __ids(unsigned inst) { IE = 0; } 			// OK. 
void __rfi(unsigned inst) { PC = IPC; IE = 1; } // OK.

/* Predefined functions
 * */
void (*operation[])(unsigned) = {
    __addi1, __addi2, __ori1,  __ori2,  \
    __andi1, __andi2, __movi1, __movi2, \
    __add,   __sub,   __not,   __neg,   \
    __or,    __and,   __xor,   __asr,   \
    __lsr,   __shl,   __ror,   __br,    \
    __brl,   __j,     __jl,    __ld,    \
    __ldr,   __st,    __str,   __lea,   \
    __nop,   __ien,   __ids,   __rfi
};

void process(unsigned int inst) { operation[I2OP_](inst); }
