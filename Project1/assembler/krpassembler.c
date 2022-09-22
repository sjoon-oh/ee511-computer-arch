#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define	IMEM_SIZE	4*1024
#define	L_OPC		27
#define	L_RA		22
#define	L_RB		17
#define	L_RC		12
#define	L_IMM10		7
#define	L_MODE		5
#define	L_I			5

#define	N_ADDI1		0
#define	N_ADDI2		0

char c_line[50];
char c_opc[20];
char c_a[20];
char c_b[20];
char c_c[20];

void parse(char* s);
unsigned int decode();

FILE *fp_w;

int main (int argc, char** argv) {
	int inc;
	FILE *fp;
	FILE *fp_wb;
	unsigned int insn;

	inc = 0;
	//fp=fopen("input_asm", "r");
	fp=fopen(argv[1], "r");
	if(fp == NULL) {
		printf("Cannot open the file.\n");
		exit(1);
	}

	fp_w=fopen("output_log", "w");
	if(fp_w == NULL) {
		printf("Cannot open the file.\n");
		exit(1);
	}

	fp_wb=fopen("vectors.bin", "wb");
	if(fp_w == NULL) {
		printf("Cannot open the file.\n");
		exit(1);
	}

	while(inc < IMEM_SIZE) {
		//fscanf(fp, "%s", c_line);
		fgets(c_line, sizeof(c_line), fp);
		if(feof(fp)) break;

		parse(c_line);
		fprintf(fp_w, "%d. ", inc+1);
		insn = decode();
		//printf("%02X", (insn&255));
		//printf("%02X", ((insn>>8)&255));
		//printf("%02X", ((insn>>16)&255));
		//printf("%02X", ((insn>>24)&255));
		fputc((char)(insn&255), fp_wb);
		fputc((char)((insn>>8)&255), fp_wb);
		fputc((char)((insn>>16)&255), fp_wb);
		fputc((char)((insn>>24)&255), fp_wb);
		inc++;

		// reset characters
		memset(c_opc, 0, 20);
		memset(c_a, 0, 20);
		memset(c_b, 0, 20);
		memset(c_c, 0, 20);
	}

	fclose(fp);
	fclose(fp_w);
	fclose(fp_wb);

	return 0;
}

void parse(char* s) {
	int i;
	int par = 0;
	int pos = 0;

	for (i = 0; i < 50; i++) {
		// opcode
		if (par == 0) {
			if ((s[i] != ' ') && (s[i] != '\t')) {
			  if (s[i] == '\n') break;
				c_opc[pos] = s[i];
				pos++;
			} else {
				pos = 0;
				par++;
				//printf("opc: %s\n", c_opc);
			}
		} 
		// a
		else if (par == 1) {
			if (s[i] == '\n') {
				//printf("a: %s\n", c_a);
				break;
			} else if ((s[i] != ' ') && (s[i] != '\t')) {
				if (s[i] != ',') {
					c_a[pos] = s[i];
					pos++;
				}
			} else {
				pos = 0;
				par++;
				//printf("a: %s\n", c_a);
			}
		}
		// b
		else if (par == 2) {
			if (s[i] == '\n') {
			   //printf("b: %s\n", c_b);
			   break;
			} else if ((s[i] != ' ') && (s[i] != '\t')) {
				if (s[i] != ',') {
					c_b[pos] = s[i];
					pos++;
				}
			} else {
				pos = 0;
				par++;
				//printf("b: %s\n", c_b);
			}
		}
		// c
		else {
			if (s[i] != '\n') {
				c_c[pos] = s[i];
				pos++;
			} else {
				pos = 0;
				par = 0;
				break;
				//printf("c: %s\n", c_c);
			}
		}

		//printf("%d: %c\n", i, s[i]);
	}
}

unsigned int decode() {
	unsigned int opcode;
	unsigned int ra, rb, rc;
	unsigned int imm10, imm17, imm22;
	unsigned int mode, shamt, cond;
	unsigned int i_value;
	char* tmp_s; 
	char t_a[20]; 
	char* t_b;
	char* t_ab;
	int pos, i;

	unsigned int result = 0;

	if (strcmp(c_opc, "ADDI") == 0) {
		if (c_c[0] == '#') { 
			opcode = 0;  
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //imm hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        imm17 = strtol(&c_c[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_c[1]);
      //imm17 = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|imm17;
			fprintf(fp_w, "ADDI type 1\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d\n", ra, rb, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else { 
			opcode = 1; 
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			if (c_c[0] == 'S' && c_c[1] == 'H' && c_c[2] == 'L') {
				mode = 0;
			} else if (c_c[0] == 'L' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 1;
			} else if (c_c[0] == 'A' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 2;
			} else if (c_c[0] == 'R' && c_c[1] == 'O' && c_c[2] == 'R') {
				mode = 3;
			} else {
				printf("Wrong shift operation: %c%c%c\n", c_c[0], c_c[1], c_c[2]);
				exit(0);
			}
			for (pos = 0, i = 4; i < 20; i++) {
				if (c_c[i] != ',') {
					t_a[pos] = c_c[i];
					pos++;
				} else {
					t_b = &c_c[i+2];
					break;
				}
			}
      //imm hex conversion
      if(t_a[1] == '-' && t_a[2] == '0' && t_a[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_a[1] == '0' && t_a[2] == 'x'){
        imm10 = strtol(&t_a[1], NULL, 16);
        if((imm10/1024)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm10 = atoi(&t_a[1]);
      //imm10 = atoi(&t_a[1]);
      //shamt hex conversion
      if(t_b[1] == '-' && t_b[2] == '0' && t_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_b[1] == '0' && t_b[2] == 'x'){
        shamt = strtol(&t_b[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&t_b[1]);
			//shamt = atoi(&t_b[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm10 = imm10 & ((1<<10)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(imm10<<L_IMM10|(mode<<L_MODE)|shamt);
			fprintf(fp_w, "ADDI type 2\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d, mode: %d, shamt: %d\n", ra, rb, imm10, mode, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "ANDI") == 0) {
		if (c_c[0] == '#') { 
			opcode = 4; 
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //imm hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        imm17 = strtol(&c_c[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_c[1]);
			//imm17 = atoi(&c_c[1]);
			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|imm17;
			fprintf(fp_w, "ANDI type 1\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d\n", ra, rb, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else { 
			opcode = 5; 
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			if (c_c[0] == 'S' && c_c[1] == 'H' && c_c[2] == 'L') {
				mode = 0;
			} else if (c_c[0] == 'L' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 1;
			} else if (c_c[0] == 'A' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 2;
			} else if (c_c[0] == 'R' && c_c[1] == 'O' && c_c[2] == 'R') {
				mode = 3;
			} else {
				printf("Wrong shift operation: %c%c%c\n", c_c[0], c_c[1], c_c[2]);
				exit(0);
			}
			for (pos = 0, i = 4; i < 20; i++) {
				if (c_c[i] != ',') {
					t_a[pos] = c_c[i];
					pos++;
				} else {
					t_b = &c_c[i+2];
					break;
				}
			}
      //imm hex conversion
      if(t_a[1] == '-' && t_a[2] == '0' && t_a[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_a[1] == '0' && t_a[2] == 'x'){
        imm10 = strtol(&t_a[1], NULL, 16);
        if((imm10/1024)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm10 = atoi(&t_a[1]);
			//imm10 = atoi(&t_a[1]);
      //shamt hex conversion
      if(t_b[1] == '-' && t_b[2] == '0' && t_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_b[1] == '0' && t_b[2] == 'x'){
        shamt = strtol(&t_b[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&t_b[1]);
			//shamt = atoi(&t_b[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm10 = imm10 & ((1<<10)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(imm10<<L_IMM10|(mode<<L_MODE)|shamt);
			fprintf(fp_w, "ANDI type 2\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d, mode: %d, shamt: %d\n", ra, rb, imm10, mode, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "ORI") == 0) {
		if (c_c[0] == '#') { 
			opcode = 2; 
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //imm hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        imm17 = strtol(&c_c[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_c[1]);
			//imm17 = atoi(&c_c[1]);
			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|imm17;
			fprintf(fp_w, "ORI type 1\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d\n", ra, rb, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else { 
			opcode = 3; 
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			if (c_c[0] == 'S' && c_c[1] == 'H' && c_c[2] == 'L') {
				mode = 0;
			} else if (c_c[0] == 'L' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 1;
			} else if (c_c[0] == 'A' && c_c[1] == 'S' && c_c[2] == 'R') {
				mode = 2;
			} else if (c_c[0] == 'R' && c_c[1] == 'O' && c_c[2] == 'R') {
				mode = 3;
			} else {
				printf("Wrong shift operation: %c%c%c\n", c_c[0], c_c[1], c_c[2]);
				exit(0);
			}
			for (pos = 0, i = 4; i < 20; i++) {
				if (c_c[i] != ',') {
					t_a[pos] = c_c[i];
					pos++;
				} else {
					t_b = &c_c[i+2];
					break;
				}
			}
      //imm hex conversion
      if(t_a[1] == '-' && t_a[2] == '0' && t_a[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_a[1] == '0' && t_a[2] == 'x'){
        imm10 = strtol(&t_a[1], NULL, 16);
        if((imm10/1024)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm10 = atoi(&t_a[1]);
			//imm10 = atoi(&t_a[1]);
      //shamt hex conversion
      if(t_b[1] == '-' && t_b[2] == '0' && t_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_b[1] == '0' && t_b[2] == 'x'){
        shamt = strtol(&t_b[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&t_b[1]);
			//shamt = atoi(&t_b[1]);
			//printf("t_a: %s, t_b: %s, shamt: %d\n", t_a, t_b, shamt);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			imm10 = imm10 & ((1<<10)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(imm10<<L_IMM10|(mode<<L_MODE)|shamt);
			fprintf(fp_w, "ORI type 2\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d, mode: %d, shamt: %d\n", ra, rb, imm10, mode, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "MOVI") == 0) {
		if (c_b[0] == '#') { 
			opcode = 6; 
			ra = atoi(&c_a[1]);
      //imm hex conversion
      if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_b[1] == '0' && c_b[2] == 'x'){
        imm17 = strtol(&c_b[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_b[1]);
			//imm17 = atoi(&c_b[1]);

			ra = ra & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|imm17;
			fprintf(fp_w, "MOVI type 1\n");
			fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else { 
			opcode = 7; 
			ra = atoi(&c_a[1]);
			if (c_b[0] == 'S' && c_b[1] == 'H' && c_b[2] == 'L') {
				mode = 0;
			} else if (c_b[0] == 'L' && c_b[1] == 'S' && c_b[2] == 'R') {
				mode = 1;
			} else if (c_b[0] == 'A' && c_b[1] == 'S' && c_b[2] == 'R') {
				mode = 2;
			} else if (c_b[0] == 'R' && c_b[1] == 'O' && c_b[2] == 'R') {
				mode = 3;
			} else {
				printf("Wrong shift operation: %c%c%c\n", c_b[0], c_b[1], c_b[2]);
				exit(0);
			}

			t_ab = &(c_b[4]);
			t_b = &(c_c[0]);
      //imm hex conversion
      if(t_ab[1] == '-' && t_ab[2] == '0' && t_ab[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_ab[1] == '0' && t_ab[2] == 'x'){
        imm10 = strtol(&t_ab[1], NULL, 16);
        if((imm10/1024)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm10 = atoi(&t_ab[1]);
			//imm10 = atoi(&t_ab[1]);
      //shamt hex conversion
      if(t_b[1] == '-' && t_b[2] == '0' && t_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(t_b[1] == '0' && t_b[2] == 'x'){
        shamt = strtol(&t_b[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&t_b[1]);
			//shamt = atoi(&t_b[1]);
			//printf("t_ab: %s, t_b: %s, imm10: %d, shamt: %d\n", t_ab, t_b, imm10, shamt);

			ra = ra & ((1<<5)-1);
			imm10 = imm10 & ((1<<10)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(imm10<<L_IMM10|(mode<<L_MODE)|shamt);
			fprintf(fp_w, "MOVI type 2\n");
			fprintf(fp_w, "\tra: %d, imm: %d, mode: %d, shamt: %d\n", ra, imm10, mode, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "ADD") == 0) {
		opcode = 8;
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		rc = atoi(&c_c[1]);

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC);
		fprintf(fp_w, "ADD\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "SUB") == 0) {
		opcode = 9;
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		rc = atoi(&c_c[1]);

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC);
		fprintf(fp_w, "SUB\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "NEG") == 0) {
		opcode = 11;
		ra = atoi(&c_a[1]);
		rc = atoi(&c_b[1]);

		ra = ra & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rc<<L_RC);
		fprintf(fp_w, "NEG\n");
		fprintf(fp_w, "\tra: %d, rc: %d\n", ra, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "NOT") == 0) {
		opcode = 10;
		ra = atoi(&c_a[1]);
		rc = atoi(&c_b[1]);

		ra = ra & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rc<<L_RC);
		fprintf(fp_w, "NOT\n");
		fprintf(fp_w, "\tra: %d, rc: %d\n", ra, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "AND") == 0) {
		opcode = 13;
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		rc = atoi(&c_c[1]);

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC);
		fprintf(fp_w, "AND\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "OR") == 0) {
		opcode = 12;
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		rc = atoi(&c_c[1]);

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC);
		fprintf(fp_w, "OR\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "XOR") == 0) {
		opcode = 14;
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		rc = atoi(&c_c[1]);

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC);
		fprintf(fp_w, "XOR\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "LSR") == 0) {
		opcode = 16;
		if (c_c[0] == '#') { 
			i_value = 0;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //shamt hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        shamt = strtol(&c_c[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&c_c[1]);
			//shamt = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(i_value<<L_I)|shamt;
			fprintf(fp_w, "LSR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, shamt: %d\n", ra, rb, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			i_value = 1;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			rc = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			rc = rc & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC)|(i_value<<L_I);
			fprintf(fp_w, "LSR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "ASR") == 0) {
		opcode = 15;
		if (c_c[0] == '#') { 
			i_value = 0;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //shamt hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        shamt = strtol(&c_c[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&c_c[1]);
			//shamt = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(i_value<<L_I)|shamt;
			fprintf(fp_w, "ASR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, shamt: %d\n", ra, rb, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			i_value = 1;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			rc = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			rc = rc & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC)|(i_value<<L_I);
			fprintf(fp_w, "ASR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "SHL") == 0) {
		opcode = 17;
		if (c_c[0] == '#') { 
			i_value = 0;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //shamt hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        shamt = strtol(&c_c[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&c_c[1]);
			//shamt = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(i_value<<L_I)|shamt;
			fprintf(fp_w, "SHL\n");
			fprintf(fp_w, "\tra: %d, rb: %d, shamt: %d\n", ra, rb, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			i_value = 1;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			rc = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			rc = rc & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC)|(i_value<<L_I);
			fprintf(fp_w, "SHL\n");
			fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "ROR") == 0) {
		opcode = 18;
		if (c_c[0] == '#') { 
			i_value = 0;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //shamt hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        shamt = strtol(&c_c[1], NULL, 16);
        if((shamt/32)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else shamt = atoi(&c_c[1]);
			//shamt = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			shamt = shamt & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(i_value<<L_I)|shamt;
			fprintf(fp_w, "ROR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, shamt: %d\n", ra, rb, shamt);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			i_value = 1;
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
			rc = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			rc = rc & ((1<<5)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC)|(i_value<<L_I);
			fprintf(fp_w, "ROR\n");
			fprintf(fp_w, "\tra: %d, rb: %d, rc: %d\n", ra, rb, rc);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (c_opc[0] == 'B' && c_opc[1] == 'R' && (c_opc[2] != 'L' || (c_opc[2] == 'L' && c_opc[3] == 'T'))) {
		opcode = 19;
		if (c_opc[2] == 'N' && c_opc[3] == 'V') { cond = 0; }
		else if (c_opc[2] == 'A' && c_opc[3] == 'L') { cond = 1; }
		else if (c_opc[2] == 'E' && c_opc[3] == 'Q') { cond = 2; }
		else if (c_opc[2] == 'N' && c_opc[3] == 'E') { cond = 3; }
		else if (c_opc[2] == 'G' && c_opc[3] == 'E') { cond = 4; }
		else if (c_opc[2] == 'L' && c_opc[3] == 'T') { cond = 5; }
		else { printf("Wrong BR condition: %c%c\n", c_opc[2], c_opc[3]); exit(0); }
		rb = atoi(&c_a[1]);
		if (c_b[0] == 'r') rc = atoi(&c_b[1]);
		else rc = 0;

		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(rb<<L_RB)|(rc<<L_RC)|cond;
		fprintf(fp_w, "BR\n");
		fprintf(fp_w, "\trb: %d, rc: %d, cond: %d\n", rb, rc, cond);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (c_opc[0] == 'B' && c_opc[1] == 'R' && c_opc[2] == 'L') {
		opcode = 20;
		if (c_opc[3] == 'N' && c_opc[4] == 'V') { cond = 0; }
		else if (c_opc[3] == 'A' && c_opc[4] == 'L') { cond = 1; }
		else if (c_opc[3] == 'E' && c_opc[4] == 'Q') { cond = 2; }
		else if (c_opc[3] == 'N' && c_opc[4] == 'E') { cond = 3; }
		else if (c_opc[3] == 'G' && c_opc[4] == 'E') { cond = 4; }
		else if (c_opc[3] == 'L' && c_opc[4] == 'T') { cond = 5; }
		else { printf("Wrong BRL condition: %c%c\n", c_opc[3], c_opc[4]); exit(0); }
		ra = atoi(&c_a[1]);
		rb = atoi(&c_b[1]);
		if (c_c[0] == 'r') rc = atoi(&c_c[1]);
		else rc = 0;

		ra = ra & ((1<<5)-1);
		rb = rb & ((1<<5)-1);
		rc = rc & ((1<<5)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|(rc<<L_RC)|cond;
		fprintf(fp_w, "BRL\n");
		fprintf(fp_w, "\tra: %d, rb: %d, rc: %d, cond: %d\n", ra, rb, rc, cond);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "J") == 0) {
		opcode = 21;
    //imm hex conversion
    if(c_a[1] == '-' && c_a[2] == '0' && c_a[3] == 'x'){
      printf("Hex input error\n");
    }
    if(c_a[1] == '0' && c_a[2] == 'x'){
      imm22 = strtol(&c_a[1], NULL, 16);
      if((imm22/4194304)!=0){
        printf("Hex range error\n");
        exit(0);
      }
    }
    else imm22 = atoi(&c_a[1]);
		//imm22 = atoi(&c_a[1]);
		imm22 = imm22 & ((1<<22)-1);
		result = (opcode<<L_OPC)|imm22;
		fprintf(fp_w, "J\n");
		fprintf(fp_w, "\timm: %d\n", imm22);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "JL") == 0) {
		opcode = 22;
		ra = atoi(&c_a[1]);
    //imm hex conversion
    if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
      printf("Hex input error\n");
    }
    if(c_b[1] == '0' && c_b[2] == 'x'){
      imm22 = strtol(&c_b[1], NULL, 16);
      if((imm22/4194304)!=0){
        printf("Hex range error\n");
        exit(0);
      }
    }
    else imm22 = atoi(&c_b[1]);
		//imm22 = atoi(&c_b[1]);
		ra = ra & ((1<<5)-1);
		imm22 = imm22 & ((1<<22)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|imm22;
		fprintf(fp_w, "JL\n");
		fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm22);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "LD") == 0) {
		opcode = 23;
		if (c_b[0] == '#') {
			ra = atoi(&c_a[1]);
      //imm hex conversion
      if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_b[1] == '0' && c_b[2] == 'x'){
        imm17 = strtol(&c_b[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_b[1]);
			//imm17 = atoi(&c_b[1]);

			ra = ra & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(31<<L_RB)|imm17;
			fprintf(fp_w, "LD (absolute)\n");
			fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //imm hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        imm17 = strtol(&c_c[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_c[1]);
			//imm17 = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			if (rb == 31) {
				printf("rb can't be 31 in LD/ST\n");
				exit(0);
			}
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|imm17;
			fprintf(fp_w, "LD (displacement)\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d\n", ra, rb, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "LDR") == 0) {
		opcode = 24;
		ra = atoi(&c_a[1]);
    //imm hex conversion
    if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
      printf("Hex input error\n");
    }
    if(c_b[1] == '0' && c_b[2] == 'x'){
      imm22 = strtol(&c_b[1], NULL, 16);
      if((imm22/4194304)!=0){
        printf("Hex range error\n");
        exit(0);
      }
    }
    else imm22 = atoi(&c_b[1]);
		//imm22 = atoi(&c_b[1]);
		ra = ra & ((1<<5)-1);
		imm22 = imm22 & ((1<<22)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|imm22;
		fprintf(fp_w, "LDR\n");
		fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm22);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "ST") == 0) {
		opcode = 25;
		if (c_b[0] == '#') {
			ra = atoi(&c_a[1]);
      //imm hex conversion
      if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_b[1] == '0' && c_b[2] == 'x'){
        imm17 = strtol(&c_b[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_b[1]);
			//imm17 = atoi(&c_b[1]);

			ra = ra & ((1<<5)-1);
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(31<<L_RB)|imm17;
			fprintf(fp_w, "ST (absolute)\n");
			fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		} else {
			ra = atoi(&c_a[1]);
			rb = atoi(&c_b[1]);
      //imm hex conversion
      if(c_c[1] == '-' && c_c[2] == '0' && c_c[3] == 'x'){
        printf("Hex input error\n");
      }
      if(c_c[1] == '0' && c_c[2] == 'x'){
        imm17 = strtol(&c_c[1], NULL, 16);
        if((imm17/131072)!=0){
          printf("Hex range error\n");
          exit(0);
        }
      }
      else imm17 = atoi(&c_c[1]);
			//imm17 = atoi(&c_c[1]);

			ra = ra & ((1<<5)-1);
			rb = rb & ((1<<5)-1);
			if (rb == 31) {
				printf("rb can't be 31 in LD/ST\n");
				exit(0);
			}
			imm17 = imm17 & ((1<<17)-1);
			result = (opcode<<L_OPC)|(ra<<L_RA)|(rb<<L_RB)|imm17;
			fprintf(fp_w, "ST (displacement)\n");
			fprintf(fp_w, "\tra: %d, rb: %d, imm: %d\n", ra, rb, imm17);
			fprintf(fp_w, "\tinsn: %08X\n", result);
		}
	} else if (strcmp(c_opc, "STR") == 0) {
		opcode = 26;
		ra = atoi(&c_a[1]);
    //imm hex conversion
    if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
      printf("Hex input error\n");
    }
    if(c_b[1] == '0' && c_b[2] == 'x'){
      imm22 = strtol(&c_b[1], NULL, 16);
      if((imm22/4194304)!=0){
        printf("Hex range error\n");
        exit(0);
      }
    }
    else imm22 = atoi(&c_b[1]);
		//imm22 = atoi(&c_b[1]);
		ra = ra & ((1<<5)-1);
		imm22 = imm22 & ((1<<22)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|imm22;
		fprintf(fp_w, "STR\n");
		fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm22);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "LEA") == 0) {
		opcode = 27;
		ra = atoi(&c_a[1]);
    //imm hex conversion
    if(c_b[1] == '-' && c_b[2] == '0' && c_b[3] == 'x'){
      printf("Hex input error\n");
    }
    if(c_b[1] == '0' && c_b[2] == 'x'){
      imm22 = strtol(&c_b[1], NULL, 16);
      if((imm22/4194304)!=0){
        printf("Hex range error\n");
        exit(0);
      }
    }
    else imm22 = atoi(&c_b[1]);
		//imm22 = atoi(&c_b[1]);
		ra = ra & ((1<<5)-1);
		imm22 = imm22 & ((1<<22)-1);
		result = (opcode<<L_OPC)|(ra<<L_RA)|imm22;
		fprintf(fp_w, "LEA\n");
		fprintf(fp_w, "\tra: %d, imm: %d\n", ra, imm22);
		fprintf(fp_w, "\tinsn: %08X\n", result);
	//} else if (strcmp(c_opc, "MSN") == 0) {
	//	opcode = 28;
	//	ra = atoi(&c_a[1]);
	//	ra = ra & ((1<<5)-1);
	//	result = (opcode<<L_OPC)|(ra<<L_RA);
	//	fprintf(fp_w, "MSN\n");
	//	fprintf(fp_w, "\tra: %d\n", ra);
	//	fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "IEN") == 0) {
		opcode = 29;
		result = (opcode<<L_OPC);
		fprintf(fp_w, "IEN\n");
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "IDS") == 0) {
		opcode = 30;
		result = (opcode<<L_OPC);
		fprintf(fp_w, "IDS\n");
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else if (strcmp(c_opc, "RFI") == 0) {
		opcode = 31;
		result = (opcode<<L_OPC);
		fprintf(fp_w, "RFI\n");
		fprintf(fp_w, "\tinsn: %08X\n", result);
	} else {
		printf("Wrong opcode: %s\n", c_opc);
		exit(0);
	}
	return result;
}
