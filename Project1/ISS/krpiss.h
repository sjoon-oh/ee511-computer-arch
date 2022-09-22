#define	IMEM_SIZE       4*1024/*Define memory size here*/
#define	DMEM_SIZE       4*1024/*Define memory size here*/

unsigned int  reg[32];  /*General purpose registers*/
unsigned int  PC;       /*Program counter*/
unsigned int  IPC;      /*Interrupted PC*/
unsigned int  IE;       /*Interrupt enable*/
unsigned int  BP;       /*Break point*/
unsigned char*  pm;     /*Program memory*/
unsigned char*  dm;     /*Data memory*/

FILE* fp_dump;          /*Pointer for dumping dm*/

/*Functions*/
void view_reg(unsigned int inst);
int init_mem(char *file_name);
void dump_data_mem(char *file_name);
void process(unsigned int inst);
