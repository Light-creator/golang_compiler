#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define INSTR_COUNT 2048
#define MEM_SZ 1024
#define BUFF_SZ 256

typedef enum {
  R1 = 0, 
  R2, R3, R4, R5, R6, R7, R8, COUNT_REGS
} regs_t;

typedef enum {
  OP_CMP, OP_MOV, OP_JZ, OP_JMP, OP_INC
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
  
  // flags
  int zero_flag;
  int neg_flag;

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

vm_t vm = { 0 };
state_t state = { 0 };

label_t* find_label(char* label) {
  int l = strlen(label);
  if(label[l-1] == ':') l--;

  for(int i=0; i<state.labels_sz; i++) {
    if(strncmp(label, state.labels[i].name, l) == 0) {
      return &state.labels[i];
    }
  }

  return NULL;
}

void fill_all_jmps() {
  for(int i=0; i<state.jmps_sz; i++) {
    if(state.jmps[i].label->instr_idx == -1) {
      printf("Undefined label: %s\n", state.jmps[i].label->name);
      exit(1);
    }
    
    printf("Found at %d\n", state.jmps[i].label->instr_idx);
    state.jmps[i].instr->a.val = state.jmps[i].label->instr_idx;
  }
}

int get_operand_value(operand_t* op) {
  switch(op->operand_type) {
    case REG:
      return vm.regs[op->val];
      break;
    case ADDR:
      return vm.mem[op->val];
      break;
    case NUM:
      return op->val;
      break;
    default:
      printf("Incorrect operand...\n");
      exit(1);
  }

  return 0;
}

void load_program(char* filename) {
  FILE* f = fopen(filename, "r");
  
  char line[BUFF_SZ] = { 0 };

  int a, b;
  char label[BUFF_SZ] = { 0 };
 
  while(fgets(line, BUFF_SZ, f)) {
      if(sscanf(line, "mov [%d], %d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_MOV; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = ADDR; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = NUM; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, "mov r%d, [%d]", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_MOV; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = ADDR; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, "mov [%d], r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_MOV; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = ADDR; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, "mov r%d, %d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_MOV; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = NUM; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "cmp r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_CMP; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, ".%s:", label) == 1) {
        label_t* label_ptr = find_label(label);
        if(!label_ptr) {
          label_ptr = &state.labels[state.labels_sz];
          int l = strlen(label);
          printf("%d\n", l);
          memcpy(label_ptr->name, label, l-1);
        }

        label_ptr->instr_idx = vm.prog_sz;
        state.labels_sz++;
      } else if(sscanf(line, "jmp %s:", label) == 1) {
        // Point as OP_JMP label
        vm.prog[vm.prog_sz].opcode = OP_JMP; 
        vm.prog[vm.prog_sz].a.operand_type = LABEL; 
        vm.prog[vm.prog_sz].a.val = -1;

        label_t* label_ptr = find_label(label);
        if(!label_ptr) {
          // save jmp and save label
          label_ptr = &state.labels[state.labels_sz];
          int l = strlen(label);
          memcpy(label_ptr->name, label, l);
          state.labels[state.labels_sz].instr_idx = -1;
          state.labels_sz++;
          
          state.jmps[state.jmps_sz].label = label_ptr;
          state.jmps[state.jmps_sz].instr = &vm.prog[vm.prog_sz];
          state.jmps_sz++;
        } else {
          vm.prog[vm.prog_sz].a.val = label_ptr->instr_idx; 
        }

        vm.prog_sz++;
      } else if(sscanf(line, "jz %s:", label) == 1) {
        // Point as OP_JMP label
        vm.prog[vm.prog_sz].opcode = OP_JZ; 
        vm.prog[vm.prog_sz].a.operand_type = LABEL; 
        vm.prog[vm.prog_sz].a.val = -1;

        label_t* label_ptr = find_label(label);
        if(!label_ptr) {
          // save jmp and save label
          label_ptr = &state.labels[state.labels_sz];
          int l = strlen(label);
          memcpy(label_ptr->name, label, l);
          state.labels[state.labels_sz].instr_idx = -1;
          state.labels_sz++;
          
          state.jmps[state.jmps_sz].label = label_ptr;
          state.jmps[state.jmps_sz].instr = &vm.prog[vm.prog_sz];
          state.jmps_sz++;
        } else {
          vm.prog[vm.prog_sz].a.val = label_ptr->instr_idx; 
        }

        vm.prog_sz++;
      } else if(sscanf(line, "inc r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_INC; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if (sscanf(line, "add r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_CMP; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "add r%d, %d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_CMP; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = NUM; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      }
  }

  fill_all_jmps();
  printf("vm.prog_sz: %d\n", vm.prog_sz);
  
  fclose(f);
}

void fetch_and_execute() {

  // fetch
  instr_t* instr = &vm.prog[vm.ip];
  sleep(1);

  printf("ip: %d\n", vm.ip);
  switch(instr->opcode) {
    case OP_JMP:
      printf("jmp\n");
      vm.ip = instr->a.val;
      break;
    case OP_JZ:
      printf("jz: %d\n", instr->a.val);
      if(vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_INC:
      printf("inc r%d\n", instr->a.val);
      vm.regs[instr->a.val]++;
      vm.ip++;
      break;
    case OP_MOV:
      printf("mov\n");
      if(instr->a.operand_type == REG) {
        vm.regs[instr->a.val] = get_operand_value(&instr->b);
      } else if(instr->a.operand_type == ADDR) {
        vm.mem[instr->a.val] = get_operand_value(&instr->b);
      }
      vm.ip++;
      break;
    case OP_CMP: {
      int a_val = get_operand_value(&instr->a);
      int b_val = get_operand_value(&instr->b);
      int cmp = a_val - b_val;
      printf("cmp(%d, %d) = %d\n", a_val, b_val, cmp);
      if(cmp == 0) {
        vm.zero_flag = 1;
        vm.neg_flag = 0;
      } else if(cmp < 0) {
        vm.neg_flag = 1;
        vm.zero_flag = 0;
      }
      vm.ip++;
      break;
                 }
    default: 
      vm.ip++;
      break;
    
  }

}

void execute_program() {
  
  while(vm.ip != vm.prog_sz) {
      fetch_and_execute();
  }

}


int main() {
    
  load_program("out.asm");
  execute_program();

  return 0;
}
