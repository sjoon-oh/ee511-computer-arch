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
 
void process(unsigned int inst) {
  /////////////////////////////////
  // Write your ISS program here //
  /////////////////////////////////
}
