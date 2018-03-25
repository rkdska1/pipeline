#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_LENGTH 32

#define R_type 0x0
//R type
#define add 0x20
#define addu 0x21
#define and 0x24
#define jr 0x08
#define nor 0x27
#define or 0x25
#define slt 0x2A
#define sltu 0x2B
#define sll 0x00
#define srl 0x02
#define sub 0x22
#define subu 0x23
#define div 0x1A
#define divu 0x1B
#define mfhi 0x10
#define mflo 0x12
//#define mfc0 
#define mult 0x18
#define multu 0x19
#define sra 0x03

//I type
#define addi 0x08
#define addiu 0x09
#define andi 0x0C
#define beq 0x04
#define bne 0x05
#define lbu 0x24
#define lhu 0x25
#define ll 0x30
#define lui 0x0F
#define lw 0x23
#define ori 0x0D
#define slti 0x0A
#define sltiu 0x0B
#define sb 0x28
#define sc 0x38
#define sh 0x29
#define sw 0x2B
#define lwc1 0x31
#define ldc1 0x33
#define swc1 0x39
#define sdc1 0x3D
//J type
#define J 0x02
#define jal 0x03
#define inuse -25000
#define isfree 25001




typedef struct {
	unsigned pc;
} pc_state_s, *pc_state;

typedef struct {
	unsigned pc;
	unsigned instr;
	unsigned NOP;
} if_id_s, *if_id;

typedef struct {
	unsigned pc;
	unsigned instr;
	int rs_v;
	int rt_v;
	unsigned unc_jump;
	unsigned jump_branch;
	unsigned NOP;
} id_ex_s, *id_ex;

typedef struct {
	unsigned pc;
	unsigned branch_target;
	unsigned jump_branch;
	int ALU_out;
	int rt_v;
	unsigned wb_dest;
	unsigned M_write;
	unsigned M_read;
	unsigned byte_count;
	unsigned NOP;
	int rd_back;
	int rt_back;
	unsigned b;
} ex_mem_s, *ex_mem;

typedef struct {
	unsigned pc;
	unsigned M_read;
	unsigned wb_dest;
	int ALU_out;
	unsigned read_data;
	unsigned NOP;
} mem_wb_s, *mem_wb;

int buffer[0xFFFFF]= {0, };
int registers[MAX_LENGTH] = {0, };
int reg_inuse[MAX_LENGTH] = {0, };
char *Registers[MAX_LENGTH] =
{"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0","s1","s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"};

pc_state pc_now;
if_id id_in;
if_id if_in;
if_id if_out;
id_ex ex_in;
id_ex id_out;
ex_mem mem_in;
ex_mem ex_out;
mem_wb wb_in;
mem_wb mem_out;



unsigned find_op(unsigned code) { return ((code)>>26&0x3f);}
unsigned find_rs(unsigned code) {return ((code)>>21&0x1f);}
unsigned find_rt(unsigned code) {return ((code)>>16&0x1f);}
unsigned find_rd(unsigned code) {return ((code)>>11&0x1f);}
unsigned find_sa(unsigned code) {return ((code)>>6&0x1f);}
unsigned find_imm(unsigned code) {return ((code)&0xffff);}			//if type is short must be checked--- if typecasted to (unsigned short), turns to not signed extended
int find_s_imm(unsigned imm) {return ((short) imm);}
unsigned find_zeroimm(unsigned imm) {return ((imm)&0xffff) ;}
unsigned find_func(unsigned code) {return (code & 0x3f);}

void IF();
void ID();
void EXE();
void MEM();
void WB();
void initialize();
unsigned run_pipe(int a);


int main(void){
	FILE *pfile; int temppc=0; int cycle=0;
	int iSize=0; int eachinst=0;
	unsigned int result=0;
	int i;
	for(i=0; i <MAX_LENGTH; i++){
		reg_inuse[i]= isfree;		
		registers[i]=0;
	}
	registers[29] = 0x100000;
	registers[MAX_LENGTH-1] = 0xffffffff;
	pfile = fopen("C:\\input.bin", "rb");
	if (pfile == NULL){
		printf("ERROR! Cannot open the file!!");
		exit(1);
	}
	if(buffer ==NULL) { printf("Memory Error"); exit(2); }
	fread(buffer, 4, 0xFFFF, pfile);//sizeof(long*
	for(i=0; i<0xFFFF; i++){
	buffer[i] = ntohl(buffer[i]);
	}
	if( ferror (pfile)) { printf("READING ERROR"); exit(3);}
	printf("\n00000000 <main>:");
	initialize();
	cycle = run_pipe(105);
	printf("\nTOTAL CYCLE IS : %d", cycle);
	printf("\n\nRegisters[v0] is %d\n\n", registers[2]);
	fclose(pfile);
	system("pause");
	return 0;
}

void IF()	{
	unsigned pc = pc_now->pc;
//	printf("\n[o] INSTRUCTION IS FETCHED\n");
	if(if_in->NOP)
		;
//		printf("\nPC::::: %d DOES NOT CHANGE", pc);
	else if(ex_out->jump_branch){
		pc = ex_out->branch_target;
		ex_out->jump_branch=0;
		id_in->NOP=0;
//		printf("\nJUMP in EXE detected::::: JUMPING");
	printf("\nPC::::: %x    ->     %x\n", pc_now->pc, pc);
	pc_now->pc= pc+4;
	}
	else if(id_out->jump_branch){
		pc = id_out->unc_jump;
		id_out->jump_branch = 0;
		id_in->NOP=0;
//		printf("\nJUMP in ID detected::::: JUMPING");
	printf("\nPC::::: %x    ->     %x\n", pc_now->pc, pc);
	pc_now->pc= pc+4;
	}
	else{
		pc_now->pc= pc+4;
	printf("\nPC::::: %x    ->     %x\n", pc, pc_now->pc);
	}
//	printf("instruction is : %08x", buffer[pc/4]);
	if(pc==0xffffffff)
		if_out->NOP=1;
	if_out->instr = buffer[pc/4];
	if_out->pc = pc;
	if_out->NOP = if_in->NOP;
	printf("\n\n---------------------------------\n");
}		//checked

void ID()	{							//when ll, 15-0 ==0, mux doesn't work=> make rt_v to give 15-11;
	unsigned write_data=0;
	unsigned instr= if_out->instr;
	unsigned op = find_op(instr);
	int a=0;
	unsigned char flag=0;
	id_in->pc = if_out->pc;
	id_in->NOP = if_out->NOP;
	id_out->rs_v = registers[find_rs(instr)];
	id_out->rt_v = registers[find_rt(instr)];
	a= find_rs(instr);
	if(instr==0x0)
		id_in->NOP=1;
//	if(id_in->NOP)
//		printf("\n         ID IS NOT EXCUTED");
//	else
//		printf("\n[o] ID IS EXCUTING");
	if(reg_inuse[find_rs(instr)]!=isfree && reg_inuse[find_rs(instr)]!= inuse){		
		a= reg_inuse[find_rs(instr)];
		id_out->rs_v= reg_inuse[find_rs(instr)];			///
		id_in->NOP=0;	if_out->NOP=0;  if_in->NOP=0;
		flag=1;
	}
	if(reg_inuse[find_rt(instr)]!=isfree && reg_inuse[find_rt(instr)]!= inuse){
		id_out->rt_v=reg_inuse[find_rt(instr)];	////
		id_in->NOP=0;	if_out->NOP=0; if_in->NOP=0;
		flag=1;
	}
	if(op==R_type){
		if(find_func(instr) == jr){
			if(reg_inuse[find_rs(instr)]==inuse){
				if_out->NOP=1; id_in->NOP=1;  if_in->NOP = 0;
				id_out->jump_branch = 1;
//				printf("\nJR is detected in ID stage::REG is in use; halt");
			}
			else{
				id_out->unc_jump= id_out->rs_v;
				id_out->jump_branch= 1;
				id_in->NOP = 1; if_in->NOP = 0;
//				printf("\nJR is detected in ID stage::REG NOT IN use; JUMP in next stage");
			}
		}
		else if(find_func(instr) == sll || find_func(instr) == srl){
			if(reg_inuse[find_rt(instr)]==inuse){
				if_out->NOP=1; id_in->NOP=1; if_in->NOP=1;
//				printf("\nSLL /// SRL is detected in ID stage::REG is in use; halt");
			}
		}
		else{
			if(reg_inuse[find_rs(instr)]==inuse || reg_inuse[find_rt(instr)]==inuse){
				if_out->NOP=1; id_in->NOP=1; if_in->NOP = 1;
			}
		}
	}
	else if(op == J){
		id_out->unc_jump = ((((id_in->pc)+4)<26)|((instr) & 0x03ffffff))<<2;            
		id_out->jump_branch=1;
		id_in->NOP = 1; if_in->NOP = 0;
	}
	else if (op == jal){
		id_out->unc_jump = ((((id_in->pc)+4)<26)|((instr) & 0x03ffffff))<<2; 
		id_out->jump_branch =1;
		if_in->NOP = 0;
		reg_inuse[find_rd(instr)]=inuse;
	}
	else if(op == beq || op == bne){
		if((reg_inuse[find_rs(instr)]==inuse) || (reg_inuse[find_rt(instr)]==inuse)){
			if_in->NOP = 1;
		}}
	else{
		if((reg_inuse[find_rs(instr)]==inuse) && flag==0){
			if_out->NOP=1; id_in->NOP=1; if_in->NOP = 1;
		}
	}
			// =--=====-=-==========-===================-=
	if(!(id_in->NOP)){
	if(op == R_type && op != jr){
		reg_inuse[find_rd(instr)]=inuse;
//		printf("\nREG %s IN USE!!!", Registers[find_rd(instr)]);
	}
	else if((op != bne) && (op != beq) && (op != sb) && (op != sc) && (op != sh) && (op != sw) && (op != J) && (op != jal)){
		reg_inuse[find_rt(instr)]=inuse;
//				printf("\nREG %s IN USE!!!", Registers[find_rt(instr)]);
	}
	}	

	id_out->pc = id_in->pc;
	id_out->instr = instr;
	id_out->NOP= id_in->NOP;
}
//		jump in mem should have higher priority than jump in id, so
//     if(mem_in->jump_branch)
//			mem- operation
//		else if(id_in->jump_branch)
//			set id- operation => mem-jump_branch
//		when id_in->jump_branch is set, set instr to 0, as to 'nop', then turn id_in->jump_branch to 0
//      


//if unc_jump is done, set 
void EXE() {
 //jump = no alu, 
	unsigned jump_branch = 0;
	unsigned instr = id_out->instr;
	unsigned func = find_func(instr);
	unsigned opcode = find_op(instr);
	unsigned M_read = 0;
	unsigned M_write = 0;
	unsigned branch_if_zero = 0;
	unsigned branch_if_nonzero = 0;
	unsigned sa = find_sa(instr);
	int rt = id_out->rt_v;
	int rs = id_out->rs_v;
	unsigned rd = find_rd(instr);
	unsigned wb_dest=0;
	unsigned imm = find_imm(instr);
	int s_imm = find_s_imm(imm);
	int zero_imm = find_zeroimm(imm);
	ex_in->instr=id_out->instr;
	ex_out->b=0;
	ex_in->pc= id_out->pc;
	ex_out->ALU_out = 0;
	ex_out->byte_count=0xffffffff;
	ex_in->NOP = id_out->NOP;
//	if(ex_in->NOP)
//		printf("\n         EXE IS NOT EXCUTED");
//	else
//		printf("\n[o] EXE IS EXCUTING\n   ");
	if(!(ex_in->NOP)){
	switch (opcode) {
	case(R_type):		//Rtype is always executed with ALU?
		switch(func) {
		case sll:
			ex_out->ALU_out = rt << sa;
			wb_dest = rd;
//			printf("sll to R[%s] being executed", Registers[rd]);
			break;
		case srl:
			ex_out->ALU_out = rt >> sa;
			wb_dest = rd;
//			printf("srl R[%s] being executed", Registers[rd]);
			break;
		case jr:
			break;
		case add:
			ex_out->ALU_out =(unsigned)(rs + rt);
			wb_dest = rd;
//			printf("add R[%s]=R[%s] + R[%s] being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
		case addu:
			ex_out->ALU_out = rs + rt;
			wb_dest = rd;
//			printf("add R[%s]=R[%s] + R[%s] being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;
		case and:
			ex_out->ALU_out = rs & rt;
			wb_dest = rd;
//			printf("and R[%s]=R[%s] & R[%s] being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;
		case nor:
			ex_out->ALU_out = ~(rs | rt);
			wb_dest = rd;
//			printf("nor R[%s]=R[%s] R[%s] being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;
		case or:
			ex_out->ALU_out = rs | rt ;
			wb_dest = rd;
//			printf("o R[%s]=R[%s] | R[%s]r being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;
		case slt:
		case sltu:
			ex_out->ALU_out = (rs < rt) ? 1:0;
			wb_dest = rd;
//			printf("sltu being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;		
		case sub:
		case subu:
			ex_out->ALU_out = rs - rt ;
			wb_dest = rd;
//			printf("subu R[%s]=R[%s]-R[%s] being executed", Registers[rd], Registers[find_rs(instr)], Registers[find_rt(instr)]);
			break;
		default:
				printf("WRONG FUNCTION TYPE!!!!");
			instr;
			break;
		}

		reg_inuse[rd]=ex_out->ALU_out;
//		printf("\n		Reg %s is now freed", Registers[rd]);
		break;
	//I- TYPE
	case J:
		break;
	case jal:
		wb_dest=31;
		ex_out->ALU_out = ex_in->pc +8;
		reg_inuse[31]=ex_out->ALU_out;
		break;
	case beq:
		if(rs == rt){
			ex_out->branch_target = (ex_in->pc) + 4 + ((short)(s_imm) << 2);
			ex_out->jump_branch=1;
			ex_in->NOP = 1;	if_in->NOP=0; 
			id_out->NOP = 1; id_in->NOP=1;
//			printf("\n		BEQ R[%s]==R[%s] IS BEING EXECUTED:::: JUMP READY", Registers[find_rs(instr)], Registers[find_rt(instr)]);
		}
		else{
//			printf("\n      BEQ FAILED-> NO JUMP");
			ex_in->NOP=1; if_in->NOP=0; id_out->NOP=1;
		}
		break;
	case bne:
		if(rs != rt){
			ex_out->branch_target = (ex_in->pc) + 4 + ((short)(s_imm) << 2);
			ex_out->jump_branch=1;
			ex_in->NOP = 1; if_in->NOP=0;
			id_out->NOP = 1; id_in->NOP=1;
//			printf("\n		BNE IS BEING EXECUTED:::: JUMP READY Reg[%d] != Reg[%d]", find_rs(instr), find_rt(instr));
		}
		else{
//			printf("\n      BNE FAILED-> NO JUMP");
			ex_in->NOP=1; if_in->NOP=0; id_out->NOP=1;
		}
		break;
	case addi:
		ex_out->ALU_out = rs +s_imm;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("addiu R[%s]=R[%s] being executed", Registers[wb_dest], Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case addiu:
		ex_out->ALU_out = (unsigned)(rs +s_imm);
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("addiu R[%s]=R[%s] being executed", Registers[wb_dest], Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case slti:
		ex_out->ALU_out = (rs < s_imm)? 1:0;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("sltiu R[%s] < s_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
	case sltiu:
		ex_out->ALU_out = ((unsigned)rs < (unsigned)s_imm)? 1:0;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("sltiu R[%s] < s_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case andi:
		ex_out->ALU_out = rs & zero_imm;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("andi R[%s] & zero_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case lui:
		ex_out->ALU_out = imm<<16;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("lui being executed");
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case ori:
		ex_out->ALU_out = rs | zero_imm;
		wb_dest = find_rt(instr);
		reg_inuse[find_rt(instr)]=ex_out->ALU_out;
//		printf("sltiu R[%s] | zero_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case lw:
		ex_out->ALU_out = rs + s_imm;
		M_read = 1;
		wb_dest = find_rt(instr);
		reg_inuse[wb_dest]=inuse;
//		printf("lw being executed");
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case lbu:
		ex_out->ALU_out = rs +s_imm; 
		ex_out->byte_count = 0xff;
		wb_dest = find_rt(instr);
		M_read =1;
		reg_inuse[wb_dest]=inuse;
		ex_out->b=1;
//		printf("lbu R[%s] + s_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case lhu:
		ex_out->ALU_out = rs + s_imm;
		ex_out->byte_count = 0xffff;
		wb_dest = find_rt(instr);
		M_read = 1;
		reg_inuse[wb_dest]=inuse;
		ex_out->b=1;
//		printf("lhu R[%s] + s_imm being executed", Registers[find_rs(instr)]);
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case sb:
		ex_out->ALU_out =rs+s_imm; 
		M_write = 1;
		ex_out->rt_v = rt & 0xff;	
//		printf("sb R[%s] + s_imm being executed", Registers[find_rs(instr)]);
		break;												//no idea
	case sh:
		ex_out->ALU_out = rs+s_imm;
		M_write = 1;
		ex_out->rt_v = rt & 0xffff;
//		printf("sh R[%s] < s_imm being executed", Registers[find_rs(instr)]);
		break;
	case sw:
		ex_out->ALU_out = rs + s_imm;
		M_write = 1;
		ex_out->rt_v = rt;
//		printf("sw being executed");
		break;
	case ll:
		ex_out->ALU_out = rs+ s_imm;
		M_read = 1;
		wb_dest= find_rt(instr);
		reg_inuse[wb_dest]=inuse;
//		printf("ll being executed");
//		printf("\n		Reg %s is now freed", Registers[find_rt(instr)]);
		break;
	case sc:									
		ex_out->ALU_out = rs+ s_imm;
		M_write = 1;
		ex_out->rt_v = rt;
		rt= (1)? 1:0;
//		printf("sc being executed");
		break;
	default:
		printf("WRONG DATA!!!!! EXITING");
		break;
	}
	}
	ex_out->M_write = M_write;
	ex_out->pc= ex_in->pc;
	ex_out->M_read = M_read;
	ex_out->wb_dest = wb_dest;
	ex_out->NOP = ex_in->NOP;
} 



void MEM()
{
	int address=0; int sdata=0;	int data = 0;
	mem_in->b = ex_out->b;
	mem_in->pc= ex_out->pc;
	mem_in->NOP = ex_out->NOP;
	mem_in->rt_v= ex_out->rt_v;
	mem_in->ALU_out = ex_out->ALU_out;
	mem_in->M_read = ex_out->M_read;
	mem_in->M_write = ex_out->M_write;
	mem_in->wb_dest= ex_out->wb_dest;
	sdata = mem_in->rt_v;
	address = mem_in->ALU_out;
//	if(mem_in->NOP)
//		printf("\n         MEM IS NOT EXCUTED");
//	else
//		printf("\n[o] MEM IS EXCUTING");
	if (mem_in->M_read){
		data = buffer[address/4];
		if(ex_out->b==1)
			data= (ex_out->byte_count) & (mem_in->ALU_out);
//		printf("\n		Reading value of %x from MEM[%d] to %d", buffer[address/4], address, data);
		reg_inuse[mem_in->wb_dest]=data;
		mem_in->NOP=0;
	}
	if (mem_in->M_write){
		buffer[address/4] = sdata;
		printf("\nWriting value of %x to MEM[%d]",sdata, address/4);
		mem_in->NOP= 1;
	}
	mem_out->M_read = mem_in->M_read;
	mem_out->wb_dest = mem_in->wb_dest;
	mem_out->ALU_out = mem_in->ALU_out;
	mem_out->read_data = data;
	mem_out->NOP = mem_in->NOP;
	mem_out->pc = mem_in->pc;
}

void WB()
{
	wb_in->NOP = mem_out->NOP;
	wb_in->read_data = mem_out->read_data;
	wb_in->M_read = mem_out->M_read;
	wb_in->wb_dest= mem_out->wb_dest;
	wb_in->ALU_out = mem_out->ALU_out;
	if(!(wb_in->NOP)){
//		printf("\n[o] WB IS EXCUTING");
		if(wb_in->M_read){
			registers[wb_in->wb_dest] = wb_in->read_data;
			printf("\nREG[%s] = %d", Registers[wb_in->wb_dest], wb_in->read_data);
		}
		else{
			registers[wb_in->wb_dest] = wb_in->ALU_out;
			printf("\nREG[%s] = %d", Registers[wb_in->wb_dest], wb_in->ALU_out);
		}
		reg_inuse[find_rt(wb_in->wb_dest)]=isfree;
	}
//	else
//		printf("\n         WB IS NOT EXCUTED");
}

void initialize()
{
	pc_now = (pc_state )malloc(sizeof(pc_state_s));
	id_in = (if_id )malloc(sizeof(if_id_s));
	if_in = (if_id )malloc(sizeof(if_id_s));
	if_out= (if_id )malloc(sizeof(if_id_s));
	ex_in = (id_ex )malloc(sizeof(id_ex_s));
	id_out = (id_ex )malloc(sizeof(id_ex_s));
	ex_out = (ex_mem )malloc(sizeof(ex_mem_s));
	mem_in = (ex_mem )malloc(sizeof(ex_mem_s));
	mem_out = (mem_wb )malloc(sizeof(mem_wb_s));
	wb_in = (mem_wb )malloc(sizeof(mem_wb_s));

	pc_now->pc = 0x0; if_in->instr=0; if_in->NOP=0; if_in->pc=0;
	if_out->pc=0; if_out->instr=0; id_in->pc=0; id_in->instr=0; if_out->NOP=1;
	id_out->instr=0; id_out->jump_branch=0; id_out->NOP=1; id_out->pc=0; id_out->rs_v=0; id_out->rt_v=0; id_out->unc_jump=0; id_in->NOP=1;
	ex_in->instr=0; ex_in->jump_branch =0; ex_in->NOP =1; ex_in->pc=0; ex_in->rs_v=0; ex_in->rt_v=0; ex_in->unc_jump=0; ex_out->b=0;
	ex_out->ALU_out=0; ex_out->branch_target=0; ex_out->byte_count=0; ex_out->jump_branch=0; ex_out->M_read=0; ex_out->M_write=0; ex_out->NOP=1; ex_out->pc=0; ex_out->rt_v=0; ex_out->wb_dest=0; ex_out->rd_back=0; ex_out->rt_back=0;
	mem_in->ALU_out=0; mem_in->branch_target=0; mem_in->byte_count=0; mem_in->jump_branch=0; mem_in->M_read=0; mem_in->M_write=0; mem_in->NOP=1; mem_in->pc=0; mem_in->rt_v=0; mem_in->wb_dest=0; mem_in->rd_back=0; mem_in->rt_back=0;
	mem_out->ALU_out=0; mem_out->M_read=0; mem_out->NOP=1; mem_out->read_data=0; mem_out->wb_dest=0;
	wb_in->ALU_out=0; wb_in->M_read=0; wb_in->NOP=1; wb_in->read_data=0; wb_in->wb_dest=0;
}


unsigned run_pipe(int a)
{
	int count=a; int i; int y;
	unsigned cycle = 0;
	while ((mem_out->pc)-4 != 0xffffffff && (mem_out->pc)-4 != 1) {
	printf("=============================\nCYCLE :  %d\n=============================\n", cycle);
		WB(); 
		MEM(); 
		EXE(); 
		ID(); 
		IF();  
	cycle++;

//	for(y=0;y<32; y++)
//		printf("R[%s] = %-12d", Registers[y], registers[y]);
	}
	for(y=0;y<32; y++)
		printf("R[%s] = %-12d", Registers[y], registers[y]);
	return cycle;
}