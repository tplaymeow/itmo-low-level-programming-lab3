#include "handlers.h"
#include "models_serialization.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const enum database_where_logic_operator
    where_logic_operator_from_model[] = {
        [SQL_LOGIC_BINARY_OPERATOR_AND] = DATABASE_WHERE_LOGIC_OPERATOR_AND,
        [SQL_LOGIC_BINARY_OPERATOR_OR] = DATABASE_WHERE_LOGIC_OPERATOR_OR,
};

static const enum database_attribute_type attribute_type_from_model[] = {
    [SQL_DATA_TYPE_INTEGER] = DATABASE_ATTRIBUTE_INTEGER,
    [SQL_DATA_TYPE_FLOATING_POINT] = DATABASE_ATTRIBUTE_FLOATING_POINT,
    [SQL_DATA_TYPE_BOOLEAN] = DATABASE_ATTRIBUTE_BOOLEAN,
    [SQL_DATA_TYPE_TEXT] = DATABASE_ATTRIBUTE_STRING,
};

static const enum database_where_comparison_operator
    where_comparison_operator_from_model[] = {
        [SQL_COMPARISON_OPERATOR_EQUAL] =
            DATABASE_WHERE_COMPARISON_OPERATOR_EQUAL,
        [SQL_COMPARISON_OPERATOR_NOT_EQUAL] =
            DATABASE_WHERE_COMPARISON_OPERATOR_NOT_EQUAL,
        [SQL_COMPARISON_OPERATOR_GREATER] =
            DATABASE_WHERE_COMPARISON_OPERATOR_GREATER,
        [SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL] =
            DATABASE_WHERE_COMPARISON_OPERATOR_GREATER_OR_EQUAL,
        [SQL_COMPARISON_OPERATOR_LESS] =
            DATABASE_WHERE_COMPARISON_OPERATOR_LESS,
        [SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL] =
            DATABASE_WHERE_COMPARISON_OPERATOR_LESS_OR_EQUAL,
};

static union database_attribute_value
attribute_value_from_literal(struct sql_literal literal) {
  switch (literal.type) {
  case SQL_DATA_TYPE_INTEGER:
    return (union database_attribute_value){.integer = literal.value.integer};
  case SQL_DATA_TYPE_FLOATING_POINT:
    return (union database_attribute_value){.floating_point =
                                                literal.value.floating_point};
  case SQL_DATA_TYPE_BOOLEAN:
    return (union database_attribute_value){.boolean = literal.value.boolean};
  case SQL_DATA_TYPE_TEXT:
    return (union database_attribute_value){.string = literal.value.text};
  }
}

static struct database_attribute
database_attribute_make(struct sql_column_with_type column) {
  return (struct database_attribute){
      .name = column.name, .type = attribute_type_from_model[column.type]};
}

char *handle_create_request(struct database *database,
                            struct sql_create_statement statement) {
  size_t columns_count = 0;
  for (struct sql_column_with_type_list *l = statement.columns; l != NULL;
       l = l->next) {
    columns_count++;
  }

  struct database_create_table_request create_request =
      database_create_table_request_create(statement.table_name, columns_count);

  size_t column_index = 0;
  for (struct sql_column_with_type_list *l = statement.columns; l != NULL;
       l = l->next) {
    const struct database_attribute attribute =
        database_attribute_make(l->item);
    database_create_table_request_set_attribute(create_request, column_index++,
                                                attribute);
  }

  const struct database_create_table_result result =
      database_create_table(database, create_request);
  if (!result.success) {
    database_create_table_request_destroy(create_request);
    return serialize_common_response((struct sql_common_response){"Failure"});
  }

  database_create_table_request_destroy(create_request);
  return serialize_common_response((struct sql_common_response){"Success"});
}

char *handle_drop_request(struct database *database,
                          struct sql_drop_statement statement) {
  const struct database_get_table_result get_table_result =
      database_get_table_with_name(database, statement.table_name);
  if (!get_table_result.success) {
    return serialize_common_response(
        (struct sql_common_response){"Table not found"});
  }

  const struct database_drop_table_result drop_table_result =
      database_drop_table(database, get_table_result.table);
  if (!drop_table_result.success) {
    database_table_destroy(get_table_result.table);
    return serialize_common_response((struct sql_common_response){"Failure"});
  }

  return serialize_common_response((struct sql_common_response){"Success"});
}

char *handle_insert_request(struct database *database,
                            struct sql_insert_statement statement) {
  const struct database_get_table_result get_table_result =
      database_get_table_with_name(database, statement.table_name);
  if (!get_table_result.success) {
    return serialize_common_response(
        (struct sql_common_response){"Table not found"});
  }

  struct database_insert_row_request request =
      database_insert_row_request_create(get_table_result.table);

  size_t column_index = 0;
  for (struct sql_literal_list *l = statement.values; l != NULL; l = l->next) {
    if (column_index >= get_table_result.table.attributes.count) {
      database_insert_row_request_destroy(request);
      database_table_destroy(get_table_result.table);
      return serialize_common_response(
          (struct sql_common_response){"Incorrect values count"});
    }

    const struct database_attribute attribute = database_attributes_get(
        get_table_result.table.attributes, column_index);
    const union database_attribute_value value =
        attribute_value_from_literal(l->item);

    if (attribute_type_from_model[l->item.type] == attribute.type) {
      database_insert_row_request_set_value(request, column_index++, value);
    } else {
      database_table_destroy(get_table_result.table);
      database_insert_row_request_destroy(request);
      return serialize_common_response(
          (struct sql_common_response){"Wrong type"});
    }
  }

  if (column_index != get_table_result.table.attributes.count) {
    database_table_destroy(get_table_result.table);
    database_insert_row_request_destroy(request);
    return serialize_common_response(
        (struct sql_common_response){"Incorrect values count"});
  }

  const struct database_insert_row_result insert_row_result =
      database_insert_row(database, get_table_result.table, request);
  if (!insert_row_result.success) {
    database_table_destroy(get_table_result.table);
    database_insert_row_request_destroy(request);
    return serialize_common_response((struct sql_common_response){"Failure"});
  }

  database_table_destroy(get_table_result.table);
  database_insert_row_request_destroy(request);
  return serialize_common_response((struct sql_common_response){"Success"});
}

static union database_attribute_value
attribute_value_make(struct sql_literal value) {
  switch (value.type) {
  case SQL_DATA_TYPE_INTEGER:
    return (union database_attribute_value){.integer = value.value.integer};
  case SQL_DATA_TYPE_FLOATING_POINT:
    return (union database_attribute_value){.floating_point =
                                                value.value.floating_point};
  case SQL_DATA_TYPE_BOOLEAN:
    return (union database_attribute_value){.boolean = value.value.boolean};
  case SQL_DATA_TYPE_TEXT:
    return (union database_attribute_value){.string = value.value.text};
  default:
    assert("Unknown default");
    return (union database_attribute_value){};
  }
}

char *
database_where_contains_item_make(struct database_where_contains_item *item_ret,
                                  struct database_table table,
                                  struct sql_text_operand operand) {
  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    item_ret->type = DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE;
    item_ret->value.attribute.attribute_position = SIZE_MAX;
    for (size_t i = 0; i < table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.attribute_position < SIZE_MAX) {
      return NULL;
    }

    return serialize_common_response(
        (struct sql_common_response){"Column not found"});
  }
  case SQL_OPERAND_TYPE_LITERAL: {
    item_ret->type = DATABASE_WHERE_CONTAINS_ITEM_CONSTANT;
    item_ret->value.constant.value = operand.value.literal;
    return NULL;
  }
  }
}

char *database_where_joined_contains_item_make(
    struct database_where_joined_contains_item *item_ret,
    struct database_table left_table, struct database_table right_table,
    struct sql_text_operand operand) {
  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    item_ret->type = DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE;
    item_ret->value.attribute.table_position = SIZE_MAX;

    for (size_t i = 0; i < left_table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(left_table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->value.attribute.table_position = 0;
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.table_position < SIZE_MAX) {
      return NULL;
    }

    for (size_t i = 0; i < right_table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(right_table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->value.attribute.table_position = 1;
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.table_position < SIZE_MAX) {
      return NULL;
    }

    return serialize_common_response(
        (struct sql_common_response){"Column not found"});
  }
  case SQL_OPERAND_TYPE_LITERAL: {
    item_ret->type = DATABASE_WHERE_CONTAINS_ITEM_CONSTANT;
    item_ret->value.constant.value = operand.value.literal;
    return NULL;
  }
  }
}

char *database_where_comparison_item_make(
    struct database_where_comparison_item *item_ret,
    struct database_table table, struct sql_operand operand) {
  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    item_ret->type = DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE;
    item_ret->value.attribute.attribute_position = SIZE_MAX;

    for (size_t i = 0; i < table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->data_type = attribute.type;
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.attribute_position < SIZE_MAX) {
      return NULL;
    }

    return serialize_common_response(
        (struct sql_common_response){"Column not found"});
  }
  case SQL_OPERAND_TYPE_LITERAL: {
    item_ret->type = DATABASE_WHERE_COMPARISON_ITEM_CONSTANT;
    item_ret->data_type = attribute_type_from_model[operand.value.literal.type];
    item_ret->value.constant.value =
        attribute_value_make(operand.value.literal);
    return NULL;
  }
  }
}

char *database_where_joined_comparison_item_make(
    struct database_where_joined_comparison_item *item_ret,
    struct database_table left_table, struct database_table right_table,
    struct sql_operand operand) {
  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    item_ret->type = DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE;
    item_ret->value.attribute.table_position = SIZE_MAX;

    for (size_t i = 0; i < left_table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(left_table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->data_type = attribute.type;
        item_ret->value.attribute.table_position = 0;
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.table_position < SIZE_MAX) {
      return NULL;
    }

    for (size_t i = 0; i < right_table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(right_table.attributes, i);
      if (strcmp(attribute.name, operand.value.column) == 0) {
        item_ret->data_type = attribute.type;
        item_ret->value.attribute.table_position = 1;
        item_ret->value.attribute.attribute_position = i;
        break;
      }
    }
    if (item_ret->value.attribute.table_position < SIZE_MAX) {
      return NULL;
    }

    return serialize_common_response(
        (struct sql_common_response){"Column not found"});
  }
  case SQL_OPERAND_TYPE_LITERAL: {
    item_ret->type = DATABASE_WHERE_COMPARISON_ITEM_CONSTANT;
    item_ret->data_type = attribute_type_from_model[operand.value.literal.type];
    item_ret->value.constant.value =
        attribute_value_make(operand.value.literal);
    return NULL;
  }
  }
}

char *database_where_make(struct database_where *join_res,
                          struct database_table table,
                          struct sql_filter filter) {
  switch (filter.type) {
  case SQL_FILTER_TYPE_ALL: {
    join_res->type = DATABASE_WHERE_TYPE_ALWAYS;
  } break;

  case SQL_FILTER_TYPE_COMPARISON: {
    join_res->type = DATABASE_WHERE_TYPE_COMPARISON;
    join_res->value.comparison.operator=
        where_comparison_operator_from_model[filter.value.comparison.operator];
    char *left_error = database_where_comparison_item_make(
        &join_res->value.comparison.left, table, filter.value.comparison.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error = database_where_comparison_item_make(
        &join_res->value.comparison.right, table,
        filter.value.comparison.right);
    if (right_error != NULL) {
      return right_error;
    }

    if (join_res->value.comparison.left.data_type !=
        join_res->value.comparison.right.data_type) {
      return serialize_common_response(
          (struct sql_common_response){"Where incorrect types"});
    }
  } break;

  case SQL_FILTER_TYPE_CONTAINS: {
    join_res->type = DATABASE_WHERE_TYPE_CONTAINS;
    char *left_error = database_where_contains_item_make(
        &join_res->value.contains.left, table, filter.value.contains.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error = database_where_contains_item_make(
        &join_res->value.contains.right, table, filter.value.contains.right);
    if (right_error != NULL) {
      return right_error;
    }
  } break;

  case SQL_FILTER_TYPE_LOGIC: {
    join_res->type = DATABASE_WHERE_TYPE_LOGIC;
    join_res->value.logic.operator=
        where_logic_operator_from_model[filter.value.logic.operator];
    join_res->value.logic.left = malloc(sizeof(struct database_where_joined));
    join_res->value.logic.right = malloc(sizeof(struct database_where_joined));
    char *left_error = database_where_make(join_res->value.logic.left, table,
                                           *filter.value.logic.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error = database_where_make(join_res->value.logic.right, table,
                                            *filter.value.logic.right);
    if (right_error != NULL) {
      return right_error;
    }
  } break;
  }

  return NULL;
}

char *database_where_joined_make(struct database_where_joined *join_res,
                                 struct database_table left_table,
                                 struct database_table right_table,
                                 struct sql_filter filter) {
  switch (filter.type) {
  case SQL_FILTER_TYPE_ALL: {
    join_res->type = DATABASE_WHERE_TYPE_ALWAYS;
  } break;

  case SQL_FILTER_TYPE_COMPARISON: {
    join_res->type = DATABASE_WHERE_TYPE_COMPARISON;
    join_res->value.comparison.operator=
        where_comparison_operator_from_model[filter.value.comparison.operator];
    char *left_error = database_where_joined_comparison_item_make(
        &join_res->value.comparison.left, left_table, right_table,
        filter.value.comparison.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error = database_where_joined_comparison_item_make(
        &join_res->value.comparison.right, left_table, right_table,
        filter.value.comparison.right);
    if (right_error != NULL) {
      return right_error;
    }

    if (join_res->value.comparison.left.data_type !=
        join_res->value.comparison.right.data_type) {
      return serialize_common_response(
          (struct sql_common_response){"Where incorrect types"});
    }
  } break;

  case SQL_FILTER_TYPE_CONTAINS: {
    join_res->type = DATABASE_WHERE_TYPE_CONTAINS;
    char *left_error = database_where_joined_contains_item_make(
        &join_res->value.contains.left, left_table, right_table,
        filter.value.contains.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error = database_where_joined_contains_item_make(
        &join_res->value.contains.right, left_table, right_table,
        filter.value.contains.right);
    if (right_error != NULL) {
      return right_error;
    }
  } break;

  case SQL_FILTER_TYPE_LOGIC: {
    join_res->type = DATABASE_WHERE_TYPE_LOGIC;
    join_res->value.logic.operator=
        where_logic_operator_from_model[filter.value.logic.operator];
    join_res->value.logic.left = malloc(sizeof(struct database_where_joined));
    join_res->value.logic.right = malloc(sizeof(struct database_where_joined));
    char *left_error =
        database_where_joined_make(join_res->value.logic.left, left_table,
                                   right_table, *filter.value.logic.left);
    if (left_error != NULL) {
      return left_error;
    }
    char *right_error =
        database_where_joined_make(join_res->value.logic.right, left_table,
                                   right_table, *filter.value.logic.right);
    if (right_error != NULL) {
      return right_error;
    }
  } break;
  }

  return NULL;
}

static struct sql_literal
sql_literal_make(struct database_attribute attribute,
                 union database_attribute_value value) {
  switch (attribute.type) {
  case DATABASE_ATTRIBUTE_INTEGER:
    return (struct sql_literal){.type = SQL_DATA_TYPE_INTEGER,
                                .value.integer = value.integer};
  case DATABASE_ATTRIBUTE_FLOATING_POINT:
    return (struct sql_literal){.type = SQL_DATA_TYPE_FLOATING_POINT,
                                .value.floating_point = value.floating_point};
  case DATABASE_ATTRIBUTE_BOOLEAN:
    return (struct sql_literal){.type = SQL_DATA_TYPE_BOOLEAN,
                                .value.boolean = value.boolean};
  case DATABASE_ATTRIBUTE_STRING:
    return (struct sql_literal){.type = SQL_DATA_TYPE_TEXT,
                                .value.text = strdup(value.string)};
  default:
    return (struct sql_literal){};
  }
}

char *handle_select_request(struct database *database,
                            struct sql_select_statement statement) {
  const struct database_get_table_result get_table_result =
      database_get_table_with_name(database, statement.table_name);
  if (!get_table_result.success) {
    return serialize_common_response(
        (struct sql_common_response){"Table not found"});
  }

  if (statement.join.has_value) {
    const struct database_get_table_result get_joined_table_result =
        database_get_table_with_name(database, statement.join.value.join_table);
    if (!get_joined_table_result.success) {
      database_table_destroy(get_table_result.table);
      return serialize_common_response(
          (struct sql_common_response){"Joined table not found"});
    }

    ssize_t left_attribute_position = -1;
    for (size_t i = 0; i < get_table_result.table.attributes.count; i++) {
      const struct database_attribute attribute =
          database_attributes_get(get_table_result.table.attributes, i);
      if (strcmp(attribute.name, statement.join.value.table_column) == 0) {
        left_attribute_position = (ssize_t)i;
        break;
      }
    }
    if (left_attribute_position < 0) {
      database_table_destroy(get_table_result.table);
      database_table_destroy(get_joined_table_result.table);
      return serialize_common_response(
          (struct sql_common_response){"Table attribute for join not found"});
    }

    ssize_t right_attribute_position = -1;
    for (size_t i = 0; i < get_joined_table_result.table.attributes.count;
         i++) {
      const struct database_attribute attribute =
          database_attributes_get(get_joined_table_result.table.attributes, i);
      if (strcmp(attribute.name, statement.join.value.join_table_column) == 0) {
        right_attribute_position = (ssize_t)i;
        break;
      }
    }
    if (right_attribute_position < 0) {
      database_table_destroy(get_table_result.table);
      database_table_destroy(get_joined_table_result.table);
      return serialize_common_response((struct sql_common_response){
          "Joined table attribute for join not found"});
    }

    const struct database_join join = {(size_t)left_attribute_position,
                                       (size_t)right_attribute_position};

    struct database_where_joined where;
    char *where_res = database_where_joined_make(&where, get_table_result.table,
                                                 get_joined_table_result.table,
                                                 statement.filter);
    if (where_res != NULL) {
      database_table_destroy(get_table_result.table);
      database_table_destroy(get_joined_table_result.table);
      return where_res;
    }

    struct sql_select_response_header header =
        sql_select_response_header_create(
            get_table_result.table.attributes.count +
            get_joined_table_result.table.attributes.count);
    for (size_t i = 0; i < get_table_result.table.attributes.count; i++) {
      header.columns[i] =
          database_attributes_get(get_table_result.table.attributes, i).name;
    }
    for (size_t i = 0; i < get_joined_table_result.table.attributes.count;
         i++) {
      header.columns[i + get_table_result.table.attributes.count] =
          database_attributes_get(get_joined_table_result.table.attributes, i)
              .name;
    }

    struct sql_literal_list_list *rows = NULL;
    struct database_select_join_result select_result =
        database_select_join_first(database, get_table_result.table,
                                   get_joined_table_result.table, join, where);
    while (select_result.success) {
      struct sql_literal_list *row = NULL;
      for (size_t i = 0; i < get_table_result.table.attributes.count; i++) {
        const struct database_attribute attribute =
            database_attributes_get(get_table_result.table.attributes, i);
        const union database_attribute_value value =
            database_attribute_values_get(select_result.left_row.values, i);
        row = sql_literal_list_create(sql_literal_make(attribute, value), row);
      }
      for (size_t i = 0; i < get_joined_table_result.table.attributes.count;
           i++) {
        const struct database_attribute attribute = database_attributes_get(
            get_joined_table_result.table.attributes, i);
        const union database_attribute_value value =
            database_attribute_values_get(select_result.right_row.values, i);
        row = sql_literal_list_create(sql_literal_make(attribute, value), row);
      }

      rows = sql_literal_list_list_create(row, rows);
      select_result = database_select_join_next(
          database, get_table_result.table, get_joined_table_result.table, join,
          where, select_result.left_row, select_result.right_row);
    }

    const struct sql_select_response response = {.header = header,
                                                 .rows = rows};
    char *response_string = serialize_select_response(response);

    sql_select_response_header_destroy(header);
    sql_literal_list_list_free(rows);
    database_table_destroy(get_table_result.table);
    database_table_destroy(get_joined_table_result.table);

    return response_string;
  } else {
    struct database_where where;
    char *where_res =
        database_where_make(&where, get_table_result.table, statement.filter);
    if (where_res != NULL) {
      return where_res;
    }

    struct sql_select_response_header header =
        sql_select_response_header_create(
            get_table_result.table.attributes.count);
    for (size_t i = 0; i < get_table_result.table.attributes.count; i++) {
      header.columns[i] =
          database_attributes_get(get_table_result.table.attributes, i).name;
    }

    struct sql_literal_list_list *rows = NULL;
    struct database_select_row_result select_result =
        database_select_row_first(database, get_table_result.table, where);
    while (select_result.success) {
      struct sql_literal_list *row = NULL;
      for (size_t i = 0; i < get_table_result.table.attributes.count; i++) {
        const struct database_attribute attribute =
            database_attributes_get(get_table_result.table.attributes, i);
        const union database_attribute_value value =
            database_attribute_values_get(select_result.row.values, i);
        row = sql_literal_list_create(sql_literal_make(attribute, value), row);
      }

      rows = sql_literal_list_list_create(row, rows);
      select_result = database_select_row_next(database, get_table_result.table,
                                               where, select_result.row);
    }

    const struct sql_select_response response = {.header = header,
                                                 .rows = rows};
    char *response_string = serialize_select_response(response);

    sql_select_response_header_destroy(header);
    sql_literal_list_list_free(rows);
    database_table_destroy(get_table_result.table);

    return response_string;
  }
}

char *handle_delete_request(struct database *database,
                            struct sql_delete_statement statement) {
  const struct database_get_table_result get_table_result =
      database_get_table_with_name(database, statement.table_name);
  if (!get_table_result.success) {
    return serialize_common_response(
        (struct sql_common_response){"Table not found"});
  }

  struct database_where where;
  char *where_res =
      database_where_make(&where, get_table_result.table, statement.filter);
  if (where_res != NULL) {
    database_table_destroy(get_table_result.table);
    return where_res;
  }

  struct database_select_row_result select_result;
  do {
    select_result =
        database_select_row_first(database, get_table_result.table, where);
    const struct database_remove_row_result remove_result =
        database_remove_row(database, select_result.row);
    if (!remove_result.success) {
      database_table_destroy(get_table_result.table);
      return serialize_common_response((struct sql_common_response){"Failed"});
    }
  } while (select_result.success);

  return serialize_common_response((struct sql_common_response){"Success"});
}

static struct database_attribute_values
apply_sets(struct database_table table, struct database_attribute_values values,
           struct sql_column_with_literal_list *set) {
  struct database_attribute_values result =
      database_attribute_values_create(table.attributes.count);
  for (size_t i = 0; i < table.attributes.count; i++) {
    union database_attribute_value value =
        database_attribute_values_get(values, i);
    for (struct sql_column_with_literal_list *l = set; l != NULL; l = l->next) {
      if (strcmp(database_attributes_get(table.attributes, i).name,
                 l->item.name) == 0) {
        value = attribute_value_from_literal(l->item.literal);
      }
    }
    database_attribute_values_set(result, i, value);
  }

  return result;
}

char *handle_update_request(struct database *database,
                            struct sql_update_statement statement) {
  const struct database_get_table_result get_table_result =
      database_get_table_with_name(database, statement.table_name);
  if (!get_table_result.success) {
    return serialize_common_response(
        (struct sql_common_response){"Table not found"});
  }

  struct database_where where;
  char *where_res =
      database_where_make(&where, get_table_result.table, statement.filter);
  if (where_res != NULL) {
    database_table_destroy(get_table_result.table);
    return where_res;
  }

  struct database_select_row_result select_result =
      database_select_row_first(database, get_table_result.table, where);
  while (select_result.success) {
    const struct database_attribute_values values = apply_sets(
        get_table_result.table, select_result.row.values, statement.set);
    const struct database_remove_row_result remove_result =
        database_remove_row(database, select_result.row);
    if (!remove_result.success) {
      database_table_destroy(get_table_result.table);
      return serialize_common_response((struct sql_common_response){"Failed"});
    }

    struct database_insert_row_request insert =
        database_insert_row_request_create(get_table_result.table);
    insert.values = values;

    struct database_insert_row_result insert_result =
        database_insert_row(database, get_table_result.table, insert);
    if (!insert_result.success) {
      database_table_destroy(get_table_result.table);
      return serialize_common_response((struct sql_common_response){"Failed"});
    }

    select_result =
        database_select_row_first(database, get_table_result.table, where);
  }

  return serialize_common_response((struct sql_common_response){"Success"});
}