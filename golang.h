#ifndef GOLANG_H_
#define GOLANG_H_

#define VARNAME_SZ 64
#define BUFF_SZ 256
#define WRITE_SZ 256 

typedef enum {
  DEFINE_INSTR = 0,
  COUNT_INSTRUCTIONS
} instr_type_t;

typedef struct var_t_ {
  char name[VARNAME_SZ];
  int idx;
  int sz;
  int num;
  int add_iter_num;
} var_t;

typedef struct state_t_ {
  int loop_idx;
  int if_idx;

  int loop_stack[BUFF_SZ];
  int loop_stack_idx;
} state_t;

#endif
