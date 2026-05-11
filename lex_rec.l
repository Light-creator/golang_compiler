%{
#include "go_parser.tab.h"
%}

%option noyywrap
%option yylineno

LETTER  [a-zA-Z_]
DIGIT   [0-9]
IDENT   {LETTER}({LETTER}|{DIGIT})*

%%
[ \t\r]+          ;                     /* пробелы игнорируем */
"\n"              { return NEWLINE; }   /* конец строки */

"package"         return KW_PACKAGE;
"import"          return KW_IMPORT;
"func"            return KW_FUNC;
"return"          return KW_RETURN;
"for"             return FOR;
"if"              return IF;
"else"            return ELSE;
"fmt"             return FMT_PACKAGE;
"var"             return VAR;
"Print"           return PRINT;
"Println"         return PRINTLN;
"int"             return INT_TYPE;
"string"          return STRING_TYPE;
"bool"            return BOOL_TYPE;

"("               return LPAR;
")"               return RPAR;
"{"               return LCURL;
"}"               return RCURL;

","               return COMMA;
"."               return DOT;
";"               return SEMICOLON;

"=="              return EQ;
"!="              return NOTEQ;
">="              return GREATER_OR_EQ;
"<="              return LESS_OR_EQ;
"<"               return LESS;
">"               return GREATER;

"&&"              return AND;
"||"              return OR;

":="              return DEFINE;
"="               return ASSIGN;

"++"              return INC;
"--"              return DEC;
"+="              return PLUS_EQ;
"-="              return MINUS_EQ;
"*="              return MUL_EQ;
"/="              return DIV_EQ;
"+"               return PLUS;
"-"               return MINUS;
"*"               return STAR;
"/"               return SLASH;

\"([^"]*)\"       return STRING;

{DIGIT}+          { yylval.num = atoi(yytext); return NUMBER; }

{IDENT}           { yylval.name = strdup(yytext); return IDENT; }

\/\/[^\n]*        { /* комментарий */ }

.                 { fprintf(stderr, "Lexical error (line %d): unexpected character '%s'\n", yylineno, yytext); }
%%