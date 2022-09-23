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
 *
 * Author: Sukjoon Oh, sjoon@kaist.ac.kr
 * */

#define ONES    		0xFFFFFFFF

#define NBITS_IMM17     17
#define NBITS_RB        5
#define NBITS_RA        5
#define NBITS_OPCODE	5

#define NBITS_SHAMT     5
#define NBTIS_MODE      2
#define NBITS_IMM10     10

// Type 1
#define OFFS_IMM17      0
#define OFFS_RB         (OFFS_IMM17 + NBITS_IMM17)
#define OFFS_RA        	(OFFS_RB + NBITS_RB)
#define OFFS_OPCODE     (OFFS_RA + NBITS_RA)

// Type 2
#define OFFS_SHAMT      0
#define OFFS_MODE       (OFF_SHAMT + NBITS_SHAMT)
#define OFFS_IMM10      (OFF_MODE + NBITS_MODE)

#define LSMASK_IMM17    (ONES >> (32 - NBITS_IMM17))
#define LSMASK_RB       (ONES >> (32 - NBITS_RB))
#define LSMASK_RA       (ONES >> (32 - NBITS_RA))
#define LSMASK_OPCODE   (ONES >> (32 - NBITS_OPCODE))
#define LSMASK_SHAMT    (ONES >> (32 - NBITS_SHAMT))
#define LSMASK_MODE     (ONES >> (32 - NBTIS_MODE))
#define LSMASK_IMM10    (ONES >> (32 - NBITS_IMM10))

#define SL_EXTR(inst, offs, lsmask) \
    ((inst >> offs) & lsmask) 

unsigned sign_ext(unsigned tar, unsigned sz) { 
   return ((tar >> (sz - 1)) << 31) 
       & ((~(ONES << (sz - 1))) & tar);
}

void __addi1(unsigned inst) {}
void __addi2(unsigned inst) {}
void __ori1(unsigned inst) {}
void __ori2(unsigned inst) {}
void __andi1(unsigned inst) {}
void __andi2(unsigned inst) {}
void __movi1(unsigned inst) { 
	unsigned ra = SL_EXTR(inst, OFFS_RA, LSMASK_RA);
	unsigned imm17 = SL_EXTR(inst, OFFS_IMM17, LSMASK_IMM17);

	reg[ra] = sign_ext(imm17, NBITS_IMM17); 
}
void __movi2(unsigned inst) {}
void __add(unsigned inst) {}
void __sub(unsigned inst) {}
void __not(unsigned inst) {}
void __neg(unsigned inst) {}
void __or(unsigned inst) {}
void __and(unsigned inst) {}
void __xor(unsigned inst) {}
void __asr(unsigned inst) {}
void __lsr(unsigned inst) {}
void __shl(unsigned inst) {}
void __ror(unsigned inst) {}
void __br(unsigned inst) {}
void __brl(unsigned inst) {}
void __j(unsigned inst) {}
void __jl(unsigned inst) {}
void __ld(unsigned inst) {}
void __ldr(unsigned inst) {}
void __st(unsigned inst) {}
void __str(unsigned inst) {}
void __lea(unsigned inst) {}
void __nop(unsigned inst) {}
void __ien(unsigned inst) {}
void __ids(unsigned inst) {}
void __rfi(unsigned inst) {}

void (*operation[])(unsigned) = {
    __addi1,    __addi2,    __ori1,     __ori2, \
    __andi1,    __andi2,    __movi1,    __movi2, \
    __add,      __sub,      __not,      __neg, \
    __or,       __and,      __xor,      __asr, \
    __lsr,      __shl,      __ror,      __br, \
    __brl,      __j,        __jl,       __ld, \
    __ldr,      __st,       __str,      __lea, \
    __nop,      __ien,      __ids,      __rfi
};

void process(unsigned int inst) {

    unsigned opcode = SL_EXTR(inst, OFFS_OPCODE, LSMASK_OPCODE);
    operation[opcode](inst);
}
