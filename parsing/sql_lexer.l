%option noyywrap
%option caseless

%{
#include <stdio.h>
#include <stdbool.h>
#include "logger.h"
#include "sql_parser.h"

void yyerror_lexer(const char* s);
%}

%%

"create" {return CREATE;}
"drop" {return DROP;}
"select" {return SELECT;}
"insert" {return INSERT;}
"delete" {return DELETE;}
"update" {return UPDATE;}
"table" {return TABLE;}
"from" {return FROM;}
"where" {return WHERE;}
"into" {return INTO;}
"set" {return SET;}
"integer" {return INTEGER_TYPE;}
"float" {return FLOATING_TYPE;}
"bool" {return BOOLEAN_TYPE;}
"text" {return TEXT_TYPE;}
"and" {return AND;}
"or" {return OR;}
"contains" {return CONTAINS;}
"join" {return JOIN;}
"on" {return ON;}
"exit" {return EXIT;}
"=" {return ASSIGN;}
"(" {return LEFT_BRACKET;}
")" {return RIGHT_BRACKET;}
";" {return SEMICOLON;}
"," {return COMMA;}
"==" {return COMPARISON_OPERATOR_EQUAL;}
"!=" {yylval.comparison_operator_val = SQL_COMPARISON_OPERATOR_NOT_EQUAL; return COMPARISON_OPERATOR;}
">" {yylval.comparison_operator_val = SQL_COMPARISON_OPERATOR_GREATER; return COMPARISON_OPERATOR;}
">=" {yylval.comparison_operator_val = SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL; return COMPARISON_OPERATOR;}
"<" {yylval.comparison_operator_val = SQL_COMPARISON_OPERATOR_LESS; return COMPARISON_OPERATOR;}
"<=" {yylval.comparison_operator_val = SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL; return COMPARISON_OPERATOR;}
"true" {yylval.boolean_val = true; return BOOLEAN_VAL;}
"false" {yylval.boolean_val = false; return BOOLEAN_VAL;}
[0-9]+\.[0-9]+ 	{yylval.floating_val = atof(yytext); return FLOATING_VAL;}
[0-9]+ {yylval.integer_val = atoi(yytext); return INTEGER_VAL;}
[a-zA-Z_][a-zA-Z0-9_]* {yylval.identifier_val = strdup(yytext); return IDENTIFIER;}
\"([^\\\"]|\\.)*\"  {
    size_t size = strlen(yytext);
    yylval.text_val = malloc(size - 1);
    strncpy(yylval.text_val, yytext + 1, size - 2);
    yylval.text_val[size - 2] = '\0';
    return TEXT_VAL;
}

[ \t\n] {}

. { yyerror_lexer(yytext); }

%%

void yyerror_lexer(const char* s) {
	debug("Lexer error: %s", s);
}