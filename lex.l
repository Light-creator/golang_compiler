%{

#include "golang.h"
#include "parser.tab.h"
#include <string.h>

extern state_t state;

%}

%option noyywrap
%option yylineno

LETTER  [a-zA-Z_]
DIGIT   [0-9]
IDENT   {LETTER}({LETTER}|{DIGIT})*


%%

[ \t\r]+          ;   
"\n"                { state.line_counter++; }

"package"           return KW_PACKAGE;
"import"            return KW_IMPORT;
"func"              return KW_FUNC;
"return"            return KW_RETURN;
"for"               return FOR;
"if"                return IF;
"fmt"               return FMT_PACKAGE;
"var"               return VAR;
"Print"             return PRINT;
"Println"           return PRINTLN;

"("                 return LPAR;
")"                 return RPAR;
"{"                 return LCURL;
"}"                 return RCURL;

","                 return COLON;
"."                 return DOT;
";"                 return SEMICOLON;

":="                return DEFINE;
"=="                return EQ;
"="                 return ASSIGN;

"+"                 return PLUS;
"-"                 return MINUS;
"*"                 return STAR;
"/"                 return SLASH;

"<"                 return LESS;
">"                 return GREATER;

\"([^"]*)\"         return STRING;

{DIGIT}+            {
  yylval.num = atoi(yytext);
  return NUMBER;
}

{IDENT} {
    yylval.name = strdup(yytext);
    return IDENT;
}

. {
    /* игнор неизвестного */
}

%%
