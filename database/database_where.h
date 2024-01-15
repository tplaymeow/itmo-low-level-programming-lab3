#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB3_DATABASE_WHERE_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB3_DATABASE_WHERE_H

#include "database_row.h"
#include "database_table.h"
#include <stddef.h>

#define DATABASE_WHERE_ALWAYS                                                  \
  ((struct database_where){.type = DATABASE_WHERE_TYPE_ALWAYS})

// struct database_where {
//   size_t attribute_position;
//   union database_attribute_value value;
//   enum {
//     DATABASE_WHERE_OPERATION_ALWAYS,
//     DATABASE_WHERE_OPERATION_EQUAL,
//     DATABASE_WHERE_OPERATION_LESS,
//     DATABASE_WHERE_OPERATION_GREATER,
//   } operation;
// };

// bool database_where_is_satisfied(struct database_table table,
//                                  struct database_row row,
//                                  struct database_where where);

struct database_where;

struct database_where_joined;

enum database_where_contains_item_type {
  DATABASE_WHERE_CONTAINS_ITEM_CONSTANT,
  DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE
};

union database_where_contains_item_value {
  struct {
    char *value;
  } constant;
  struct {
    size_t attribute_position;
  } attribute;
};

union database_where_joined_contains_item_value {
  struct {
    char *value;
  } constant;
  struct {
    size_t table_position;
    size_t attribute_position;
  } attribute;
};

struct database_where_contains_item {
  enum database_where_contains_item_type type;
  union database_where_contains_item_value value;
};

struct database_where_joined_contains_item {
  enum database_where_contains_item_type type;
  union database_where_joined_contains_item_value value;
};

struct database_where_contains {
  struct database_where_contains_item left;
  struct database_where_contains_item right;
};

struct database_where_joined_contains {
  struct database_where_joined_contains_item left;
  struct database_where_joined_contains_item right;
};

enum database_where_comparison_item_type {
  DATABASE_WHERE_COMPARISON_ITEM_CONSTANT,
  DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE
};

union database_where_comparison_item_value {
  struct {
    enum database_attribute_type type;
    union database_attribute_value value;
  } constant;
  struct {
    size_t attribute_position;
  } attribute;
};

union database_where_joined_comparison_item_value {
  struct {
    union database_attribute_value value;
  } constant;
  struct {
    size_t table_position;
    size_t attribute_position;
  } attribute;
};

struct database_where_comparison_item {
  enum database_where_comparison_item_type type;
  enum database_attribute_type data_type;
  union database_where_comparison_item_value value;
};

struct database_where_joined_comparison_item {
  enum database_where_comparison_item_type type;
  enum database_attribute_type data_type;
  union database_where_joined_comparison_item_value value;
};

enum database_where_comparison_operator {
  DATABASE_WHERE_COMPARISON_OPERATOR_EQUAL,
  DATABASE_WHERE_COMPARISON_OPERATOR_NOT_EQUAL,
  DATABASE_WHERE_COMPARISON_OPERATOR_GREATER,
  DATABASE_WHERE_COMPARISON_OPERATOR_GREATER_OR_EQUAL,
  DATABASE_WHERE_COMPARISON_OPERATOR_LESS,
  DATABASE_WHERE_COMPARISON_OPERATOR_LESS_OR_EQUAL
};

struct database_where_comparison {
  enum database_where_comparison_operator operator;
  struct database_where_comparison_item left;
  struct database_where_comparison_item right;
};

struct database_where_joined_comparison {
  enum database_where_comparison_operator operator;
  struct database_where_joined_comparison_item left;
  struct database_where_joined_comparison_item right;
};

enum database_where_logic_operator {
  DATABASE_WHERE_LOGIC_OPERATOR_AND,
  DATABASE_WHERE_LOGIC_OPERATOR_OR
};

struct database_where_logic {
  enum database_where_logic_operator operator;
  struct database_where *left;
  struct database_where *right;
};

struct database_where_joined_logic {
  enum database_where_logic_operator operator;
  struct database_where_joined *left;
  struct database_where_joined *right;
};

enum database_where_type {
  DATABASE_WHERE_TYPE_ALWAYS,
  DATABASE_WHERE_TYPE_LOGIC,
  DATABASE_WHERE_TYPE_COMPARISON,
  DATABASE_WHERE_TYPE_CONTAINS
};

union database_where_value {
  struct database_where_contains contains;
  struct database_where_comparison comparison;
  struct database_where_logic logic;
};

union database_where_joined_value {
  struct database_where_joined_contains contains;
  struct database_where_joined_comparison comparison;
  struct database_where_joined_logic logic;
};

struct database_where {
  enum database_where_type type;
  union database_where_value value;
};

struct database_where_joined {
  enum database_where_type type;
  union database_where_joined_value value;
};

bool database_where_is_satisfied(struct database_table table,
                                 struct database_row row,
                                 struct database_where where);

bool database_where_joined_is_satisfied(struct database_table left_table,
                                        struct database_table right_table,
                                        struct database_row left_row,
                                        struct database_row right_row,
                                        struct database_where_joined where);

void database_where_destroy(struct database_where where);

void database_where_joined_destroy(struct database_where_joined where);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB3_DATABASE_WHERE_H
