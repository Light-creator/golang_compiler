%define parse.error verbose

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "golang.h"

extern int yylineno;
int yylex(void);
void yyerror(const char *s);

FILE* out_file = NULL;

state_t state = { 0 };

int error_count = 0;
int used_var_size = 0;

var_t vars[BUFF_SZ] = { 0 };
int sz_vars = 0;

var_t* create_var(char* name, int num) {
  int name_sz = strlen(name);
  
  int var_exists = false;
  int idx = sz_vars;
  var_t* v = NULL;
  
  for(int i=0; i<sz_vars; i++) {
    if(strncmp(name, vars[i].name, name_sz) == 0) {
      idx = vars[i].idx;
      var_exists = true;
      v = &vars[i];
    }
  }

  if(!var_exists) {
    v = &vars[sz_vars++];
    memcpy(v->name, name, name_sz);
    v->scope_idx = state.g_scope_idx;
    v->is_active = true;
  } else if(!v->is_active) {
    v->scope_idx = state.g_scope_idx;
    v->is_active = true;
  } else {
    printf("Error on line %d: Variable %s is already defined!\n", state.line_counter, name);
    exit(1);
  }

  v->num = num;
  v->idx = idx;

  return v;
}

var_t* get_var(char* name) {
  int l = strlen(name);

  for(int i=0; i<sz_vars; i++) {
    if(strncmp(name, vars[i].name, l) == 0) {
      if(vars[i].is_active) return &vars[i];
    }
  }


  printf("Error on line %d: Use of undefined variable \"%s\"\n", state.line_counter, name);
  exit(1);
}
 
void write_instruction(instr_type_t type, var_t* var) {
  char buff[WRITE_SZ] = { 0 };

  switch(type) {
    case DEFINE_INSTR: {
      fprintf(out_file, "mov [%d], %d\n", var->idx,  var->num);
      break; 
    }
    default: break;
  }
}

void write_cmp_stub() {
  fprintf(out_file, "pop r3\n");
  fprintf(out_file, "pop r2\n");
  fprintf(out_file, "cmp r2, r3\n");
}

void clear_vars() {
  for(int i=0; i<sz_vars; i++) {
    if(vars[i].scope_idx == state.g_scope_idx) {
      vars[i].is_active = false;
    } 
  }
}

%}

%union {
    char *name;
    int num;
    var_t* var;
}

%token KW_PACKAGE KW_IMPORT KW_FUNC KW_RETURN
%token FOR IF VAR
%token LPAR RPAR LCURL RCURL
%token COLON DOT SEMICOLON PLUS MINUS STAR SLASH INC DEC PLUS_EQ MINUS_EQ MUL_EQ DIV_EQ
%token <name> IDENT
%token <num> NUMBER
%token STRING
%token DEFINE PRINT PRINTLN FMT_PACKAGE ASSIGN
%token EQ NOTEQ LESS GREATER GREATER_OR_EQ LESS_OR_EQ


%type <var> define redefine // for_init

%left PLUS MINUS
%left STAR SLASH
%right UMINUS
%nonassoc INC DEC 


%%

program
    : package_decl import_decl_list func
    ;

package_decl
    : KW_PACKAGE IDENT
    ;

import_decl_list
    : 
    | import_decl_list import_decl
    ;

import_decl
    : KW_IMPORT STRING
    ;

func
    : KW_FUNC IDENT LPAR RPAR block
    ;

block
    : LCURL stmt_list RCURL 
    ;

stmt_list
    : 
    | stmt_list stmt
    ;

stmt: define
    | for_loop
    | print_stmt
    | if_stmt
    | redefine
    | expr
    ;


redefine: IDENT ASSIGN expr {
          // var_t* var = create_var($1, 0);
          var_t* var = get_var($1);
          fprintf(out_file, "pop r5\n");
          fprintf(out_file, "mov [%d], r5\n", var->idx);
          $$ = var;
        }
        ;

define: IDENT DEFINE expr {
          var_t* var = create_var($1, 0);
          fprintf(out_file, "pop r5\n");
          fprintf(out_file, "mov [%d], r5\n", var->idx);
          $$ = var;
      } 
      | VAR IDENT ASSIGN expr {
          var_t* var = create_var($2, 0);
          fprintf(out_file, "pop r5\n");
          fprintf(out_file, "mov [%d], r5\n", var->idx);
          $$ = var;
      }
      ;

general_cmp: expr EQ expr {
        write_cmp_stub();
        fprintf(out_file, "jnz exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      | expr NOTEQ expr {
        write_cmp_stub();
        fprintf(out_file, "jz exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      | expr LESS expr {
        write_cmp_stub();
        fprintf(out_file, "jge exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      | expr GREATER expr {
        write_cmp_stub();
        fprintf(out_file, "jle exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      | expr GREATER_OR_EQ expr {
        write_cmp_stub();
        fprintf(out_file, "jl exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      | expr LESS_OR_EQ expr {
        write_cmp_stub();
        fprintf(out_file, "jg exit_label_%d\n", state.loop_stack[state.loop_stack_idx]); 
      }
      ;

// for_init: define {
//         $$ = $1;
//       }
//       ;

for_loop: FOR {
          state.g_scope_idx++;
          state.loop_stack[++state.loop_stack_idx] = state.loop_idx;
          state.loop_idx++;        
          
          fprintf(out_file, ".start_label_%d:\n", state.loop_stack[state.loop_stack_idx]); 
        } for_body
        ;

for_body: 
        // IDENT DEFINE {
        //   delete_last_row();
        // } expr SEMICOLON {
        //   var_t* var = create_var($1, 0);
        //   fprintf(out_file, "pop r5\n");
        //   fprintf(out_file, "mov [%d], r5\n", var->idx);
        //
        //   fprintf(out_file, ".start_label_%d:\n", state.loop_stack[state.loop_stack_idx]); 
        // } general_cmp SEMICOLON expr block {
        //   fprintf(out_file, "jmp start_label_%d\n", state.loop_stack[state.loop_stack_idx]);
        //   fprintf(out_file, ".exit_label_%d:\n", state.loop_stack[state.loop_stack_idx]);
        //   state.loop_stack_idx--;
        //
        //   clear_vars();
        //   state.g_scope_idx--;
        // }
        // | 
        general_cmp block {
          fprintf(out_file, "jmp start_label_%d\n", state.loop_stack[state.loop_stack_idx]);
          fprintf(out_file, ".exit_label_%d:\n", state.loop_stack[state.loop_stack_idx]);
          state.loop_stack_idx--;

          clear_vars();
          state.g_scope_idx--;        
        }
        ;  
        

print_stmt: FMT_PACKAGE DOT PRINT LPAR print_expr RPAR {
        fprintf(out_file, "out r4\n");
      }
      | FMT_PACKAGE DOT PRINTLN LPAR print_expr RPAR {
        fprintf(out_file, "outa r4\n");
      }
      ;

print_expr:
    | NUMBER { fprintf(out_file, "mov r4, %d\n", $1); }
    | IDENT { 
      var_t* var = get_var($1);
      fprintf(out_file, "mov r4, [%d]\n", var->idx); 
    }
    ;

if_stmt: IF {
        state.g_scope_idx++;
        
        state.loop_stack[++state.loop_stack_idx] = state.loop_idx;
        state.loop_idx++;
        fprintf(out_file, ".start_label_%d:\n", state.loop_stack[state.loop_stack_idx]); 
      } general_cmp block {
        fprintf(out_file, ".exit_label_%d:\n", state.loop_stack[state.loop_stack_idx]);
        state.loop_stack_idx--;
        
        clear_vars();
        state.g_scope_idx--;
      }
      ;


expr: expr PLUS mul {
      fprintf(out_file, "pop r6\n");
      fprintf(out_file, "pop r5\n");
      fprintf(out_file, "add r5, r6\n");
      fprintf(out_file, "push r5\n");
    }
    | expr MINUS mul {
      fprintf(out_file, "pop r6\n");
      fprintf(out_file, "pop r5\n");
      fprintf(out_file, "sub r5, r6\n");
      fprintf(out_file, "push r5\n");
    }
    | mul
    ;

mul: mul STAR term {
      fprintf(out_file, "pop r6\n");
      fprintf(out_file, "pop r5\n");
      fprintf(out_file, "imul r5, r6\n");
      fprintf(out_file, "push r5\n");
    }
    | mul SLASH term {
      fprintf(out_file, "pop r6\n");
      fprintf(out_file, "pop r5\n");
      fprintf(out_file, "idiv r5, r6\n");
      fprintf(out_file, "push r5\n");
    }
    | term 
    ;

term: NUMBER { 
        fprintf(out_file, "mov r5, %d\n", $1); 
        fprintf(out_file, "push r5\n");
      }
      | IDENT { 
        var_t* var = get_var($1);
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT INC {
        var_t* var = get_var($1);
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "push r5\n");
        fprintf(out_file, "inc r5\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
      }
      | INC IDENT {
        var_t* var = get_var($2);
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "inc r5\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT DEC {
        var_t* var = get_var($1);
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "push r5\n");
        fprintf(out_file, "dec r5\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
      }
      | DEC IDENT {
        var_t* var = get_var($2);
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "dec r5\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT PLUS_EQ expr {
        var_t* var = get_var($1);
        fprintf(out_file, "pop r6\n");
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "add r5, r6\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT MINUS_EQ expr {
        var_t* var = get_var($1);
        fprintf(out_file, "pop r6\n");
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "sub r5, r6\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT MUL_EQ expr {
        var_t* var = get_var($1);
        fprintf(out_file, "pop r6\n");
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "imul r5, r6\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | IDENT DIV_EQ expr {
        var_t* var = get_var($1);
        fprintf(out_file, "pop r6\n");
        fprintf(out_file, "mov r5, [%d]\n", var->idx);
        fprintf(out_file, "idiv r5, r6\n");
        fprintf(out_file, "mov [%d], r5\n", var->idx);
        fprintf(out_file, "push r5\n");
      }
      | LPAR expr RPAR {
        // fprintf(out_file, "mov r5, r6\n");
      }
      ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error on line %d: %s\n", yylineno, s);
    error_count++;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s file.go\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    out_file = fopen("out.asm", "w");
    if(!out_file) {
      printf("Failed to open out.asm\n");
      exit(1);
    }

    state.line_counter = 1;

    extern FILE *yyin;
    yyin = f;

    yyparse();

    fclose(f);
    fclose(out_file);

    return 0;
}


