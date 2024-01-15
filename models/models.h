#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB3_MODELS_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB3_MODELS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

enum sql_data_type {
  SQL_DATA_TYPE_INTEGER,
  SQL_DATA_TYPE_FLOATING_POINT,
  SQL_DATA_TYPE_BOOLEAN,
  SQL_DATA_TYPE_TEXT
};

struct sql_column_with_type {
  char *name;
  enum sql_data_type type;
};

struct sql_column_with_type_list {
  struct sql_column_with_type item;
  struct sql_column_with_type_list *next;
};

struct sql_column_with_type_list *
sql_column_with_type_list_create(struct sql_column_with_type item,
                                 struct sql_column_with_type_list *next);

void sql_column_with_type_list_free(struct sql_column_with_type_list *list);

union sql_literal_value {
  int64_t integer;
  double floating_point;
  bool boolean;
  char *text;
};

struct sql_literal {
  enum sql_data_type type;
  union sql_literal_value value;
};

struct sql_literal_list {
  struct sql_literal item;
  struct sql_literal_list *next;
};

struct sql_literal_list *sql_literal_list_create(struct sql_literal item,
                                                 struct sql_literal_list *next);

void sql_literal_list_free(struct sql_literal_list *list);

struct sql_literal_list_list {
  struct sql_literal_list *item;
  struct sql_literal_list_list *next;
};

struct sql_literal_list_list *
sql_literal_list_list_create(struct sql_literal_list *item,
                             struct sql_literal_list_list *next);

void sql_literal_list_list_free(struct sql_literal_list_list *list);

enum sql_operand_type { SQL_OPERAND_TYPE_COLUMN, SQL_OPERAND_TYPE_LITERAL };

union sql_operand_value {
  char *column;
  struct sql_literal literal;
};

struct sql_operand {
  enum sql_operand_type type;
  union sql_operand_value value;
};

union sql_text_operand_value {
  char *column;
  char *literal;
};

struct sql_text_operand {
  enum sql_operand_type type;
  union sql_text_operand_value value;
};

enum sql_comparison_operator {
  SQL_COMPARISON_OPERATOR_EQUAL,
  SQL_COMPARISON_OPERATOR_NOT_EQUAL,
  SQL_COMPARISON_OPERATOR_GREATER,
  SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL,
  SQL_COMPARISON_OPERATOR_LESS,
  SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL
};

struct sql_comparison {
  enum sql_comparison_operator operator;
  struct sql_operand left;
  struct sql_operand right;
};

struct sql_contains {
  struct sql_text_operand left;
  struct sql_text_operand right;
};

enum sql_logic_binary_operator {
  SQL_LOGIC_BINARY_OPERATOR_AND,
  SQL_LOGIC_BINARY_OPERATOR_OR
};

struct sql_logic {
  enum sql_logic_binary_operator operator;
  struct sql_filter *left;
  struct sql_filter *right;
};

enum sql_filter_type {
  SQL_FILTER_TYPE_ALL,
  SQL_FILTER_TYPE_COMPARISON,
  SQL_FILTER_TYPE_CONTAINS,
  SQL_FILTER_TYPE_LOGIC
};

union sql_filter_value {
  struct sql_comparison comparison;
  struct sql_contains contains;
  struct sql_logic logic;
};

struct sql_filter {
  enum sql_filter_type type;
  union sql_filter_value value;
};

struct sql_join {
  char *join_table;
  char *table_column;
  char *join_table_column;
};

struct sql_join_optional {
  bool has_value;
  struct sql_join value;
};

struct sql_column_with_literal {
  char *name;
  struct sql_literal literal;
};

struct sql_column_with_literal_list {
  struct sql_column_with_literal item;
  struct sql_column_with_literal_list *next;
};

struct sql_column_with_literal_list *
sql_column_with_literal_list_create(struct sql_column_with_literal item,
                                    struct sql_column_with_literal_list *next);

void sql_column_with_literal_list_free(
    struct sql_column_with_literal_list *list);

struct sql_create_statement {
  char *table_name;
  struct sql_column_with_type_list *columns;
};

struct sql_drop_statement {
  char *table_name;
};

struct sql_insert_statement {
  char *table_name;
  struct sql_literal_list *values;
};

struct sql_select_statement {
  char *table_name;
  struct sql_join_optional join;
  struct sql_filter filter;
};

struct sql_delete_statement {
  char *table_name;
  struct sql_filter filter;
};

struct sql_update_statement {
  char *table_name;
  struct sql_filter filter;
  struct sql_column_with_literal_list *set;
};

enum sql_statement_type {
  SQL_STATEMENT_TYPE_CREATE,
  SQL_STATEMENT_TYPE_DROP,
  SQL_STATEMENT_TYPE_INSERT,
  SQL_STATEMENT_TYPE_SELECT,
  SQL_STATEMENT_TYPE_DELETE,
  SQL_STATEMENT_TYPE_UPDATE
};

union sql_statement_value {
  struct sql_create_statement create;
  struct sql_drop_statement drop;
  struct sql_insert_statement insert;
  struct sql_select_statement select;
  struct sql_delete_statement delete;
  struct sql_update_statement update;
};

struct sql_statement {
  enum sql_statement_type type;
  union sql_statement_value value;
};

struct sql_common_response {
  char *message;
};

struct sql_select_response_header {
  size_t columns_count;
  char **columns;
};

struct sql_select_response_header
sql_select_response_header_create(size_t columns_count);

void sql_select_response_header_destroy(
    struct sql_select_response_header header);

struct sql_select_response {
  struct sql_select_response_header header;
  struct sql_literal_list_list *rows;
};

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB3_MODELS_H
