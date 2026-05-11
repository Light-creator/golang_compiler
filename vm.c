#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "vm.h"

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
    
    // printf("Found at %d\n", state.jmps[i].label->instr_idx);
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

void jmp_parse(char* label) {
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
           // printf("%d\n", l);
          memcpy(label_ptr->name, label, l-1);
        }

        label_ptr->instr_idx = vm.prog_sz;
        state.labels_sz++;
      } else if(sscanf(line, "jmp %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JMP; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jz %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JZ; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jnz %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JNZ; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jle %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JLE; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jge %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JGE; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jl %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JL; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "jg %s:", label) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_JG; 
        jmp_parse(label);
        vm.prog_sz++;
      } else if(sscanf(line, "inc r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_INC; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "dec r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_DEC; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if (sscanf(line, "add r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_ADD; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "add r%d, %d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_ADD; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = NUM; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "sub r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_SUB; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "sub r%d, %d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_SUB; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = NUM; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "imul r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_IMUL; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "idiv r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_IDIV; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if (sscanf(line, "out r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_OUT; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if (sscanf(line, "outa r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_OUTA; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "pop r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_POP; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "push r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_PUSH; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "sete r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETE; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "setne r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETNE; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "setl r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETL; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "setle r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETLE; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "setg r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETG; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "setge r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_SETGE; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "and r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_AND; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, "or r%d, r%d", &a, &b) == 2) {
        vm.prog[vm.prog_sz].opcode = OP_OR; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 

        // b operand
        vm.prog[vm.prog_sz].b.operand_type = REG; 
        vm.prog[vm.prog_sz].b.val = b; 
        vm.prog_sz++;
      } else if(sscanf(line, "neg r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_NEG; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } else if(sscanf(line, "not r%d", &a) == 1) {
        vm.prog[vm.prog_sz].opcode = OP_NOT; 

        // a operand
        vm.prog[vm.prog_sz].a.operand_type = REG; 
        vm.prog[vm.prog_sz].a.val = a; 
        vm.prog_sz++;
      } 
  }

  fill_all_jmps();
  #if DEBUG
  printf("vm.prog_sz: %d\n", vm.prog_sz);
  #endif

  fclose(f);
}

void fetch_and_execute() {

  // fetch
  instr_t* instr = &vm.prog[vm.ip];
  // sleep(1);
  #if DEBUG
  printf("ip: %d\n", vm.ip);
  #endif
  switch(instr->opcode) {
    case OP_JMP:
      #if DEBUG
      printf("jmp\n");
      #endif
      vm.ip = instr->a.val;
      break;
    case OP_JZ:
      #if DEBUG
      printf("jz: %d\n", instr->a.val);
      #endif
      if(vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_JNZ:
      #if DEBUG
      printf("jnz: %d\n", instr->a.val);
      #endif
      if(!vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_JL:
      #if DEBUG
      printf("jl: %d\n", instr->a.val);
      #endif
      if(vm.neg_flag && !vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_JG:
      #if DEBUG
      printf("jg: %d\n", instr->a.val);
      #endif
      if(!vm.neg_flag && !vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_JLE:
      #if DEBUG
      printf("jle: %d\n", instr->a.val);
      #endif
      if(vm.neg_flag || vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_JGE:
      #if DEBUG
      printf("jge: %d\n", instr->a.val);
      #endif
      if(!vm.neg_flag || vm.zero_flag) vm.ip = instr->a.val;
      else vm.ip++; 
      break;
    case OP_INC:
      #if DEBUG
      printf("inc r%d\n", instr->a.val);
      #endif
      vm.regs[instr->a.val]++;
      vm.ip++;
      break;
    case OP_DEC:
      #if DEBUG
      printf("dec r%d\n", instr->a.val);
      #endif
      vm.regs[instr->a.val]--;
      vm.ip++;
      break;
    case OP_ADD:
      #if DEBUG
      printf("add\n");
      #endif
      if(instr->b.operand_type == REG) vm.regs[instr->a.val] += vm.regs[instr->b.val];
      else if(instr->b.operand_type == NUM) vm.regs[instr->a.val] += instr->b.val;
      vm.ip++;
      break;
    case OP_SUB:
      #if DEBUG
      printf("add\n");
      #endif
      if(instr->b.operand_type == REG) vm.regs[instr->a.val] -= vm.regs[instr->b.val];
      else if(instr->b.operand_type == NUM) vm.regs[instr->a.val] -= instr->b.val;
      vm.ip++;
      break;
    case OP_IMUL:
      #if DEBUG
      printf("imul\n");
      #endif
      vm.regs[instr->a.val] *= vm.regs[instr->b.val];
      vm.ip++;
      break;
    case OP_IDIV:
      #if DEBUG
      printf("idiv\n");
      #endif
      vm.regs[instr->a.val] /= vm.regs[instr->b.val];
      vm.ip++;
      break;
    case OP_PUSH:
      #if DEBUG
      printf("push r%d\n", instr->a.val);
      #endif
      vm.stack[++vm.stack_sz] = get_operand_value(&instr->a);
      vm.ip++;
      break;
    case OP_POP:
      #if DEBUG
      printf("pop r%d = %d\n", instr->a.val, vm.stack[vm.stack_sz]);
      #endif
      vm.regs[instr->a.val] = vm.stack[vm.stack_sz--];
      vm.ip++;
      break;
    case OP_MOV:
      #if DEBUG
      printf("mov\n");
      #endif
      if(instr->a.operand_type == REG) {
        vm.regs[instr->a.val] = get_operand_value(&instr->b);
      } else if(instr->a.operand_type == ADDR) {
        vm.mem[instr->a.val] = get_operand_value(&instr->b);
      }
      vm.ip++;
      break;
    case OP_SETE:
      #if DEBUG
      printf("sete\n");
      #endif
      if(vm.zero_flag) {
        vm.regs[instr->a.val] = 1;
      }
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_SETNE:
      #if DEBUG
      printf("setne\n");
      #endif
      if(!vm.zero_flag) vm.regs[instr->a.val] = 1;
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_SETL:
      #if DEBUG
      printf("setl\n");
      #endif
      if(!vm.zero_flag && vm.neg_flag) vm.regs[instr->a.val] = 1;
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_SETLE:
      #if DEBUG
      printf("setle\n");
      #endif
      if(vm.zero_flag || vm.neg_flag) vm.regs[instr->a.val] = 1;
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_SETG:
      #if DEBUG
      printf("setg\n");
      #endif
      if(!vm.zero_flag && !vm.neg_flag) vm.regs[instr->a.val] = 1;
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_NOT:
      #if DEBUG
      printf("not\n");
      #endif
      vm.regs[instr->a.val] = vm.regs[instr->a.val] ? 0 : 1;
      vm.ip++;
      break;
    case OP_NEG:
      #if DEBUG
      printf("neg\n");
      #endif
      vm.regs[instr->a.val] = -vm.regs[instr->a.val];
      vm.ip++;
      break;
    case OP_SETGE:
      #if DEBUG
      printf("setge\n");
      #endif
      if(vm.zero_flag || !vm.neg_flag) vm.regs[instr->a.val] = 1;
      else vm.regs[instr->a.val] = 0;
      vm.ip++;
      break;
    case OP_AND:
      #if DEBUG
      printf("and\n");
      #endif
      vm.regs[instr->a.val] = (vm.regs[instr->a.val] && vm.regs[instr->b.val]);
      vm.ip++;
      break;
    case OP_OR:
      #if DEBUG
      printf("or r%d, r%d => or %d, %d\n", instr->a.val, instr->b.val, vm.regs[instr->a.val], vm.regs[instr->b.val]);
      #endif
      vm.regs[instr->a.val] = (vm.regs[instr->a.val] || vm.regs[instr->b.val]); 
      vm.ip++;
      break;
    case OP_CMP: {
      int a_val = get_operand_value(&instr->a);
      int b_val = get_operand_value(&instr->b);
      int cmp = a_val - b_val;
      #if DEBUG
      printf("cmp(%d, %d) = %d\n", a_val, b_val, cmp);
      #endif
      if(cmp == 0) {
        vm.zero_flag = 1;
        vm.neg_flag = 0;
      } else if(cmp < 0) {
        vm.neg_flag = 1;
        vm.zero_flag = 0;
      } else if(cmp > 0) {
        vm.neg_flag = 0;
        vm.zero_flag = 0;
      }
      vm.ip++;
      break;
                 }
    case OP_OUT:
      #if DEBUG
      printf("out r%d\n", instr->a.val);
      #endif
      printf("%d", vm.regs[instr->a.val]);
      vm.ip++;
      break;
    case OP_OUTA:
      #if DEBUG
      printf("outa r%d\n", instr->a.val);
      #endif
      printf("%d\n", vm.regs[instr->a.val]);
      vm.ip++;
      break;
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
