#ifndef VM_H_
#define VM_H_

#define INSTR_COUNT 2048
#define MEM_SZ 1024
#define STACK_SZ 256
#define BUFF_SZ 256

#define DEBUG 0

typedef enum {
  R1 = 0, 
  R2, R3, R4, R5, R6, R7, R8, COUNT_REGS
} regs_t;

typedef enum {
  OP_CMP, 
  OP_MOV, 
  OP_JZ, 
  OP_JNZ, 
  OP_JL, 
  OP_JG, 
  OP_JLE, 
  OP_JGE, 
  OP_JMP, 
  OP_INC, 
  OP_OUT, 
  OP_POP, 
  OP_PUSH, 
  OP_ADD, 
  OP_IMUL, 
  OP_IDIV, 
  OP_OUTA
} opcode_t;

typedef enum {
  REG, NUM, LABEL, ADDR
} operand_type_t;

typedef struct operand_t_ {
  operand_type_t operand_type;
  int val;
} operand_t;

typedef struct instr_t_ {
  opcode_t opcode;
  operand_t a;
  operand_t b;
} instr_t;

typedef struct vm_t_ {
  // instructions
  instr_t prog[INSTR_COUNT];
  int prog_sz;
  
  // memory
  int mem[MEM_SZ];
  int regs[COUNT_REGS];
  
  int stack[STACK_SZ];
  int stack_sz;
  
  // flags
  int zero_flag;
  int neg_flag;
  int carry;

  // pointers
  int ip;
} vm_t;

typedef struct label_t_ {
  char name[BUFF_SZ];
  int instr_idx;
} label_t;

typedef struct jmp_t_ {
  label_t* label;
  instr_t* instr;
} jmp_t;

typedef struct state_t_ {
  label_t labels[BUFF_SZ];
  int labels_sz;

  jmp_t jmps[BUFF_SZ];
  int jmps_sz;
} state_t;

#endif
