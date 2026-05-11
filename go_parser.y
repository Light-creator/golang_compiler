%define parse.error verbose // подробное описание ошибок

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

%token KW_PACKAGE KW_IMPORT KW_FUNC KW_RETURN
%token FOR IF ELSE VAR
%token FMT_PACKAGE PRINT PRINTLN
%token INT_TYPE STRING_TYPE BOOL_TYPE

%token LPAR RPAR LCURL RCURL
%token COMMA DOT SEMICOLON NEWLINE

%token PLUS MINUS STAR SLASH
%token INC DEC PLUS_EQ MINUS_EQ MUL_EQ DIV_EQ
%token ASSIGN DEFINE


%token EQ NOTEQ LESS GREATER GREATER_OR_EQ LESS_OR_EQ
%token AND OR

%token <name> IDENT
%token <num> NUMBER
%token STRING

%left OR
%left AND
%left EQ NOTEQ LESS GREATER GREATER_OR_EQ LESS_OR_EQ
%left PLUS MINUS
%left STAR SLASH
%right UMINUS

%%

program
    : package_decl newlines import_decl_list newlines func
    ;

newlines
    : 
    | newlines NEWLINE
    ;

package_decl
    : KW_PACKAGE IDENT NEWLINE
    ;

import_decl_list
    : 
    | import_decl_list import_decl
    ;

import_decl
    : KW_IMPORT STRING NEWLINE
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

stmt
    : simple_stmt NEWLINE
    | return_stmt NEWLINE
    | var_decl NEWLINE
    | for_stmt
    | if_stmt
    | print_stmt NEWLINE
    | NEWLINE                         
    | error NEWLINE   { yyerrok; yyclearin; }
    ;

simple_stmt
    : expr
    | assign
    | inc_dec
    ;

assign
    : IDENT ASSIGN expr
    | IDENT DEFINE expr
    | IDENT PLUS_EQ expr
    | IDENT MINUS_EQ expr
    | IDENT MUL_EQ expr
    | IDENT DIV_EQ expr
    ;

inc_dec
    : IDENT INC
    | INC IDENT
    | IDENT DEC
    | DEC IDENT
    ;

var_decl
    : VAR IDENT IDENT ASSIGN expr
    | VAR IDENT ASSIGN expr
    | VAR IDENT STRING_TYPE
    | VAR IDENT BOOL_TYPE
    | VAR IDENT INT_TYPE
    ;

return_stmt
    : KW_RETURN expr
    | KW_RETURN
    ;

print_stmt
    : FMT_PACKAGE DOT PRINT LPAR expr RPAR
    | FMT_PACKAGE DOT PRINTLN LPAR expr RPAR
    ;

for_stmt
    : FOR block
    | FOR expr block
    | FOR for_clause block
    ;

for_clause
    : simple_stmt SEMICOLON expr SEMICOLON simple_stmt
    | simple_stmt SEMICOLON expr SEMICOLON
    | SEMICOLON expr SEMICOLON simple_stmt
    | SEMICOLON expr SEMICOLON
    ;

if_stmt
    : IF expr block else_part
    ;

else_part
    : ELSE block
    | 
    ;

expr
    : expr OR expr
    | expr AND expr
    | expr EQ expr
    | expr NOTEQ expr
    | expr LESS expr
    | expr GREATER expr
    | expr GREATER_OR_EQ expr
    | expr LESS_OR_EQ expr
    | expr PLUS expr
    | expr MINUS expr
    | expr STAR expr
    | expr SLASH expr
    | MINUS expr %prec UMINUS
    | atom
    ;

atom
    : NUMBER
    | IDENT
    | IDENT LPAR arg_list RPAR
    | LPAR expr RPAR
    ;

arg_list
    : 
    | expr
    | arg_list COMMA expr
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error (line %d): %s\n", yylineno, s);
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
