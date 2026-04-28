%{
#include <stdio.h>
#include <stdlib.h>

extern int yylineno;
int yylex(void);
void yyerror(const char *s);

int error_count = 0;
%}

%union {
    char *name;
    int num;
}

/* токены */
%token KW_PACKAGE KW_IMPORT KW_FUNC KW_RETURN
%token LPAR RPAR LCURL RCURL
%token COLON DOT SEMICOLON PLUS MINUS STAR
%token <name> IDENT
%token NUMBER STRING
%token DEFINE

%%

program
    : package_decl import_decl_list func_list
    ;

package_decl
    : KW_PACKAGE IDENT
    ;

import_decl_list
    : /* пусто */
    | import_decl_list import_decl
    ;

import_decl
    : KW_IMPORT STRING
    ;

func_list
    : func
    | func_list func
    ;

func
    : KW_FUNC IDENT LPAR RPAR block
    ;

block
    : LCURL stmt_list RCURL
    ;

stmt_list
    : /* пусто */
    | stmt_list stmt
    ;

stmt
    : simple_stmt
    | return_stmt
    ;

simple_stmt
    : IDENT DEFINE expr
    | expr
    ;

return_stmt
    : KW_RETURN expr
    ;

expr
    : IDENT
    | NUMBER
    | STRING
    | expr PLUS expr
    | IDENT LPAR arg_list RPAR
    | expr DOT IDENT LPAR arg_list RPAR
    ;

arg_list
    : /* пусто */
    | expr
    | arg_list COLON expr
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Синтаксическая ошибка (строка %d): %s\n", yylineno, s);
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

    extern FILE *yyin;
    yyin = f;

    yyparse();

    if (error_count == 0)
        printf("OK\n");
    else
        printf("Not OK\n");

    fclose(f);
    return 0;
}

/*

Нужно создать язык программирования, компилятор и виртуальную машину

1. ЯП поддерживает:
  - Присваивание переменным
  - for, while
  - if
  - print
  - Целочисленные операции

2. Язык переводится в asm
  В asm есть:
  - mov
  - add, sub, mul, div
  - cmp, jmp
  - Регистры: r1 - r8
  - out

3. Волжен быть доступ к участку памяти, т.е. создаем виртуальную машину для исполнения кода.

С пунктами 1 и 2 понятно как делать и реализовывать. Но я не понимаю, как подступиться к созданию виртуальной машины. Как она должна исполнять ассемблерный код.

*/
