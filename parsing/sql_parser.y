%{
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "models.h"

extern int yylex();

void yyerror(struct sql_statement *result, const char* s);
%}

%code requires {
#include "models.h"
}

%union {
    bool boolean_val;
    double floating_val;
    int32_t integer_val;
    char *text_val;
    char *identifier_val;

    enum sql_comparison_operator comparison_operator_val;

    struct sql_statement statement_val;
    struct sql_create_statement create_statement_val;
    struct sql_drop_statement drop_statement_val;
    struct sql_insert_statement insert_statement_val;
    struct sql_select_statement select_statement_val;
    struct sql_delete_statement delete_statement_val;
    struct sql_update_statement update_statement_val;
    struct sql_column_with_type_list *column_with_type_list_val;
    struct sql_column_with_type column_with_type_val;
    enum sql_data_type data_type_val;
    struct sql_literal_list *literal_list_val;
    struct sql_literal literal_val;
    struct sql_operand operand_val;
    struct sql_comparison comparison_val;
    struct sql_logic logic_val;
    struct sql_filter filter_val;
    struct sql_column_with_literal column_with_literal_val;
    struct sql_column_with_literal_list *column_with_literal_list_val;
    struct sql_text_operand text_operand_val;
    struct sql_contains contains_val;
    struct sql_join_optional join_val;
}

%token<boolean_val> BOOLEAN_VAL
%token<floating_val> FLOATING_VAL
%token<integer_val> INTEGER_VAL
%token<text_val> TEXT_VAL
%token<identifier_val> IDENTIFIER
%token<comparison_operator_val> COMPARISON_OPERATOR
%token CREATE DROP SELECT INSERT DELETE UPDATE TABLE FROM WHERE INTO INTEGER_TYPE FLOATING_TYPE BOOLEAN_TYPE TEXT_TYPE LEFT_BRACKET RIGHT_BRACKET SEMICOLON COMMA AND OR SET ASSIGN CONTAINS JOIN ON COMPARISON_OPERATOR_EQUAL

%type<statement_val> statement
%type<create_statement_val> create_statement
%type<drop_statement_val> drop_statement
%type<insert_statement_val> insert_statement
%type<select_statement_val> select_statement
%type<delete_statement_val> delete_statement
%type<update_statement_val> update_statement
%type<column_with_type_list_val> column_with_type_list column_with_type_list_loop
%type<column_with_type_val> column_with_type
%type<data_type_val> data_type
%type<literal_list_val> literal_list literal_list_loop
%type<literal_val> literal
%type<operand_val> operand
%type<comparison_val> comparison
%type<logic_val> logic
%type<filter_val> filter where
%type<column_with_literal_val> column_with_literal
%type<column_with_literal_list_val> column_with_literal_list
%type<text_operand_val> text_operand
%type<contains_val> contains
%type<join_val> join

%start input

%parse-param { struct sql_statement *result }

%%

input
    : statement SEMICOLON {*result = $1;}
    ;

statement
    : drop_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_DROP,
            .value.drop = $1
        };
    }
    | create_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_CREATE,
            .value.create = $1
        };
    }
    | insert_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_INSERT,
            .value.insert = $1
        };
    }
    | select_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_SELECT,
            .value.select = $1
        };
    }
    | update_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_UPDATE,
            .value.update = $1
        };
    }
    | delete_statement {
        $$ = (struct sql_statement) {
            .type = SQL_STATEMENT_TYPE_DELETE,
            .value.delete = $1
        };
    }
    ;

create_statement
    : CREATE TABLE IDENTIFIER column_with_type_list {
        $$ = (struct sql_create_statement) {
            .table_name = $3,
            .columns = $4
        };
    }
    ;

drop_statement
    : DROP TABLE IDENTIFIER {
        $$ = (struct sql_drop_statement) {
            .table_name = $3
        };
    }
    ;

insert_statement
    : INSERT INTO IDENTIFIER literal_list {
        $$ = (struct sql_insert_statement) {
            .table_name = $3,
            .values = $4
        };
    }
    ;

select_statement
    : SELECT FROM IDENTIFIER join where {
        $$ = (struct sql_select_statement) {
            .table_name = $3,
            .join = $4,
            .filter = $5
        };
    }
    ;

update_statement
    : UPDATE IDENTIFIER SET column_with_literal_list where {
        $$ = (struct sql_update_statement) {
            .table_name = $2,
            .filter = $5,
            .set = $4
        };
    }

delete_statement
    : DELETE FROM IDENTIFIER where {
        $$ = (struct sql_delete_statement) {
            .table_name = $3,
            .filter = $4
        };
    }
    ;

join
    : {
        $$ = (struct sql_join_optional) {
            .has_value = false
        };
    }
    | JOIN IDENTIFIER ON IDENTIFIER COMPARISON_OPERATOR_EQUAL IDENTIFIER {
        $$ = (struct sql_join_optional) {
            .has_value = true,
            .value = (struct sql_join) {
                .join_table = $2,
                .table_column = $4,
                .join_table_column = $6
            }
        };
    }
    ;

column_with_literal_list
    : column_with_literal {
        $$ = sql_column_with_literal_list_create($1, NULL);
    }
    | column_with_literal_list COMMA column_with_literal {
        $$ = sql_column_with_literal_list_create($3, $1);
    }
    ;

column_with_literal
    : IDENTIFIER ASSIGN literal {
        $$ = (struct sql_column_with_literal) {
            .name = $1,
            .literal = $3
        };
    }
    ;

where
    : {
        $$  = (struct sql_filter) {
            .type = SQL_FILTER_TYPE_ALL
        };
    }
    | WHERE filter {$$  = $2;}
    ;

filter
    : comparison {
        $$  = (struct sql_filter) {
            .type = SQL_FILTER_TYPE_COMPARISON,
            .value.comparison = $1
        };
    }
    | contains {
        $$  = (struct sql_filter) {
            .type = SQL_FILTER_TYPE_CONTAINS,
            .value.contains = $1
        };
    }
    | logic {
        $$  = (struct sql_filter) {
            .type = SQL_FILTER_TYPE_LOGIC,
            .value.logic = $1
        };
    }
    | LEFT_BRACKET filter RIGHT_BRACKET {$$ = $2;}
    ;

logic
    : filter OR filter {
        struct sql_filter *left = malloc(sizeof(struct sql_filter)); *left = $1;
        struct sql_filter *right = malloc(sizeof(struct sql_filter)); *right = $3;
        $$ = (struct sql_logic) {
            .operator = SQL_LOGIC_BINARY_OPERATOR_OR,
            .left = left, .right = right
        };
    }
    | filter AND filter {
        struct sql_filter *left = malloc(sizeof(struct sql_filter)); *left = $1;
        struct sql_filter *right = malloc(sizeof(struct sql_filter)); *right = $3;
        $$ = (struct sql_logic) {
            .operator = SQL_LOGIC_BINARY_OPERATOR_AND,
            .left = left, .right = right
        };
    }
    ;

contains
    : text_operand CONTAINS text_operand {
        $$ = (struct sql_contains) {
            .left = $1, .right = $3
        };
    }
    ;

comparison
    : operand COMPARISON_OPERATOR operand {
        $$ = (struct sql_comparison) {
            .operator = $2,
            .left = $1,
            .right = $3
        };
    }
    | operand COMPARISON_OPERATOR_EQUAL operand {
        $$ = (struct sql_comparison) {
            .operator = SQL_COMPARISON_OPERATOR_EQUAL,
            .left = $1,
            .right = $3
        };
    }
    ;

text_operand
    : IDENTIFIER {
        $$ = (struct sql_text_operand) {
            .type = SQL_OPERAND_TYPE_COLUMN,
            .value.column = $1
        };
    }
    | TEXT_VAL {
        $$ = (struct sql_text_operand) {
            .type = SQL_OPERAND_TYPE_LITERAL,
            .value.literal = $1
        };
    }
    ;

operand
    : IDENTIFIER {
        $$ = (struct sql_operand) {
            .type = SQL_OPERAND_TYPE_COLUMN,
            .value.column = $1
        };
    }
    | literal {
        $$ = (struct sql_operand) {
            .type = SQL_OPERAND_TYPE_LITERAL,
            .value.literal = $1
        };
    }
    ;

literal_list
    : LEFT_BRACKET literal_list_loop RIGHT_BRACKET {$$ = $2;}
    ;

literal_list_loop
    : literal {
        $$ = sql_literal_list_create($1, NULL);
    }
    | literal_list_loop COMMA literal {
        $$ = sql_literal_list_create($3, $1);
    }
    ;

literal
    : INTEGER_VAL {
        $$ = (struct sql_literal) {
            .type = SQL_DATA_TYPE_INTEGER,
            .value.integer = $1
        };
    }
    | FLOATING_VAL {
        $$ = (struct sql_literal) {
            .type = SQL_DATA_TYPE_FLOATING_POINT,
            .value.floating_point = $1
        };
    }
    | BOOLEAN_VAL {
        $$ = (struct sql_literal) {
            .type = SQL_DATA_TYPE_BOOLEAN,
            .value.boolean = $1
        };
    }
    | TEXT_VAL {
        $$ = (struct sql_literal) {
            .type = SQL_DATA_TYPE_TEXT,
            .value.text = $1
        };
    }
    ;

column_with_type_list
    : LEFT_BRACKET column_with_type_list_loop RIGHT_BRACKET {$$ = $2;}
    ;

column_with_type_list_loop
    : column_with_type {
        $$ = sql_column_with_type_list_create($1, NULL);
    }
    | column_with_type_list_loop COMMA column_with_type {
        $$ = sql_column_with_type_list_create($3, $1);
    }
    ;

column_with_type
    : IDENTIFIER data_type {
        $$ = (struct sql_column_with_type) {
            .name = $1,
            .type = $2
        };
    }
    ;

data_type
    : INTEGER_TYPE {$$ = SQL_DATA_TYPE_INTEGER;}
    | FLOATING_TYPE {$$ = SQL_DATA_TYPE_FLOATING_POINT;}
    | BOOLEAN_TYPE {$$ = SQL_DATA_TYPE_BOOLEAN;}
    | TEXT_TYPE {$$ = SQL_DATA_TYPE_TEXT;}
    ;
%%

void yyerror(struct sql_statement *result, const char* s) {
	fprintf(stderr, "Parser error: %s\n", s);
	exit(1);
}