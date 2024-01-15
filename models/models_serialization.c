#include "models_serialization.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>

static const char *data_type_to_string[] = {
    [SQL_DATA_TYPE_INTEGER] = "integer",
    [SQL_DATA_TYPE_FLOATING_POINT] = "floating_point",
    [SQL_DATA_TYPE_BOOLEAN] = "boolean",
    [SQL_DATA_TYPE_TEXT] = "text",
};

static bool data_type_from_string(enum sql_data_type *ret, const char *string) {
  if (strcmp(string, "integer") == 0)
    *ret = SQL_DATA_TYPE_INTEGER;
  else if (strcmp(string, "floating_point") == 0)
    *ret = SQL_DATA_TYPE_FLOATING_POINT;
  else if (strcmp(string, "boolean") == 0)
    *ret = SQL_DATA_TYPE_BOOLEAN;
  else if (strcmp(string, "text") == 0)
    *ret = SQL_DATA_TYPE_TEXT;
  else
    return false;
  return true;
}

static const char *comparison_operator_to_string[] = {
    [SQL_COMPARISON_OPERATOR_EQUAL] = "EQUAL",
    [SQL_COMPARISON_OPERATOR_NOT_EQUAL] = "NOT_EQUAL",
    [SQL_COMPARISON_OPERATOR_GREATER] = "GREATER",
    [SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL] = "GREATER_OR_EQUAL",
    [SQL_COMPARISON_OPERATOR_LESS] = "LESS",
    [SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL] = "LESS_OR_EQUAL",
};

static bool comparison_operator_from_string(enum sql_comparison_operator *ret,
                                            const char *string) {
  if (strcmp(string, "EQUAL") == 0)
    *ret = SQL_COMPARISON_OPERATOR_EQUAL;
  else if (strcmp(string, "NOT_EQUAL") == 0)
    *ret = SQL_COMPARISON_OPERATOR_NOT_EQUAL;
  else if (strcmp(string, "GREATER") == 0)
    *ret = SQL_COMPARISON_OPERATOR_GREATER;
  else if (strcmp(string, "GREATER_OR_EQUAL") == 0)
    *ret = SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL;
  else if (strcmp(string, "LESS") == 0)
    *ret = SQL_COMPARISON_OPERATOR_LESS;
  else if (strcmp(string, "LESS_OR_EQUAL") == 0)
    *ret = SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL;
  else
    return false;
  return true;
}

static const char *logic_operator_to_string[] = {
    [SQL_LOGIC_BINARY_OPERATOR_AND] = "AND",
    [SQL_LOGIC_BINARY_OPERATOR_OR] = "OR",
};

static bool logic_operator_from_string(enum sql_logic_binary_operator *ret,
                                       const char *string) {
  if (strcmp(string, "AND") == 0)
    *ret = SQL_LOGIC_BINARY_OPERATOR_AND;
  else if (strcmp(string, "OR") == 0)
    *ret = SQL_LOGIC_BINARY_OPERATOR_OR;
  else
    return false;
  return true;
}

static cJSON *serialize_column_with_type(struct sql_column_with_type column) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "name", column.name) == NULL ||
      cJSON_AddStringToObject(result, "type",
                              data_type_to_string[column.type]) == NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_column_with_type(struct sql_column_with_type *column,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *nameJSON = cJSON_GetObjectItem(json, "name");
  const cJSON *typeJSON = cJSON_GetObjectItem(json, "type");
  if (nameJSON == NULL || typeJSON == NULL || !cJSON_IsString(nameJSON) ||
      !cJSON_IsString(typeJSON) ||
      !data_type_from_string(&column->type, typeJSON->valuestring))
    return false;

  column->name = strdup(nameJSON->valuestring);
  return true;
}

static cJSON *
serialize_column_with_type_list(struct sql_column_with_type_list *list) {
  cJSON *result = cJSON_CreateArray();
  if (result == NULL)
    return NULL;

  for (struct sql_column_with_type_list *item = list; item != NULL;
       item = item->next) {
    cJSON *item_json = serialize_column_with_type(item->item);
    if (item_json == NULL || !cJSON_AddItemToArray(result, item_json)) {
      cJSON_Delete(result);
      return NULL;
    }
  }

  return result;
}

static bool
deserialize_column_with_type_list(struct sql_column_with_type_list **list,
                                  const cJSON *json) {
  if (!cJSON_IsArray(json))
    return false;

  int count = cJSON_GetArraySize(json);
  *list = NULL;
  for (int i = 0; i < count; i++) {
    cJSON *item_json = cJSON_GetArrayItem(json, i);
    struct sql_column_with_type item;
    if (item_json == NULL || !cJSON_IsObject(item_json) ||
        !deserialize_column_with_type(&item, item_json)) {
      if (*list != NULL)
        sql_column_with_type_list_free(*list);
      return false;
    }

    *list = sql_column_with_type_list_create(item, *list);
  }

  return true;
}

static cJSON *serialize_literal(struct sql_literal literal) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "type",
                              data_type_to_string[literal.type]) == NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  switch (literal.type) {
  case SQL_DATA_TYPE_INTEGER: {
    if (cJSON_AddNumberToObject(result, "value",
                                (double)literal.value.integer) == NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_DATA_TYPE_FLOATING_POINT: {
    if (cJSON_AddNumberToObject(result, "value",
                                literal.value.floating_point) == NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_DATA_TYPE_BOOLEAN: {
    if (cJSON_AddBoolToObject(result, "value", literal.value.boolean) == NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_DATA_TYPE_TEXT: {
    if (cJSON_AddStringToObject(result, "value", literal.value.text) == NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  }

  return result;
}

static bool deserialize_literal(struct sql_literal *literal,
                                const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;
  const cJSON *typeJSON = cJSON_GetObjectItem(json, "type");
  const cJSON *valueJSON = cJSON_GetObjectItem(json, "value");
  if (typeJSON == NULL || valueJSON == NULL || !cJSON_IsString(typeJSON) ||
      !data_type_from_string(&literal->type, typeJSON->valuestring))
    return false;

  switch (literal->type) {
  case SQL_DATA_TYPE_INTEGER: {
    if (!cJSON_IsNumber(valueJSON) || valueJSON->valuedouble < INT32_MIN ||
        valueJSON->valuedouble > INT32_MAX)
      return false;
    literal->value.integer = (int32_t)valueJSON->valuedouble;
  } break;
  case SQL_DATA_TYPE_FLOATING_POINT: {
    if (!cJSON_IsNumber(valueJSON))
      return false;
    literal->value.floating_point = valueJSON->valuedouble;
  } break;
  case SQL_DATA_TYPE_BOOLEAN: {
    if (!cJSON_IsBool(valueJSON))
      return false;
    literal->value.boolean = valueJSON->valueint;
  } break;
  case SQL_DATA_TYPE_TEXT: {
    if (!cJSON_IsString(valueJSON))
      return false;
    literal->value.text = strdup(valueJSON->valuestring);
  } break;
  }
  return true;
}

static cJSON *serialize_literal_list(struct sql_literal_list *list) {
  cJSON *result = cJSON_CreateArray();
  if (result == NULL)
    return NULL;

  for (struct sql_literal_list *item = list; item != NULL; item = item->next) {
    cJSON *item_json = serialize_literal(item->item);
    if (item_json == NULL || !cJSON_AddItemToArray(result, item_json)) {
      cJSON_Delete(result);
      return NULL;
    }
  }

  return result;
}

static cJSON *serialize_literal_list_list(struct sql_literal_list_list *list) {
  cJSON *result = cJSON_CreateArray();
  if (result == NULL)
    return NULL;

  for (struct sql_literal_list_list *item = list; item != NULL;
       item = item->next) {
    cJSON *item_json = serialize_literal_list(item->item);
    if (item_json == NULL || !cJSON_AddItemToArray(result, item_json)) {
      cJSON_Delete(result);
      return NULL;
    }
  }

  return result;
}

static bool deserialize_literal_list(struct sql_literal_list **list,
                                     const cJSON *json) {
  if (!cJSON_IsArray(json))
    return false;
  int count = cJSON_GetArraySize(json);
  *list = NULL;
  for (int i = 0; i < count; i++) {
    cJSON *item_json = cJSON_GetArrayItem(json, i);
    struct sql_literal item;
    if (item_json == NULL || !cJSON_IsObject(item_json) ||
        !deserialize_literal(&item, item_json)) {
      if (*list != NULL)
        sql_literal_list_free(*list);
      return false;
    }

    *list = sql_literal_list_create(item, *list);
  }
  return true;
}

static bool deserialize_literal_list_list(struct sql_literal_list_list **list,
                                          const cJSON *json) {
  if (!cJSON_IsArray(json))
    return false;
  int count = cJSON_GetArraySize(json);
  *list = NULL;
  for (int i = 0; i < count; i++) {
    cJSON *item_json = cJSON_GetArrayItem(json, i);
    struct sql_literal_list *item;
    if (item_json == NULL || !deserialize_literal_list(&item, item_json)) {
      if (*list != NULL)
        sql_literal_list_list_free(*list);
      return false;
    }

    *list = sql_literal_list_list_create(item, *list);
  }
  return true;
}

static cJSON *serialize_operand(struct sql_operand operand) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    if (cJSON_AddStringToObject(result, "column", operand.value.column) ==
        NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_OPERAND_TYPE_LITERAL: {
    cJSON *literal = serialize_literal(operand.value.literal);
    if (literal == NULL || !cJSON_AddItemToObject(result, "literal", literal)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  }

  return result;
}

static bool deserialize_operand(struct sql_operand *operand,
                                const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;
  const cJSON *columnJSON = cJSON_GetObjectItem(json, "column");
  const cJSON *literalJSON = cJSON_GetObjectItem(json, "literal");
  if (columnJSON != NULL && literalJSON != NULL)
    return false;

  if (columnJSON != NULL) {
    if (!cJSON_IsString(columnJSON))
      return false;
    operand->type = SQL_OPERAND_TYPE_COLUMN;
    operand->value.column = strdup(columnJSON->valuestring);
  } else if (literalJSON != NULL) {
    operand->type = SQL_OPERAND_TYPE_LITERAL;
    if (!deserialize_literal(&operand->value.literal, literalJSON))
      return false;
  } else {
    return false;
  }

  return true;
}

static cJSON *serialize_text_operand(struct sql_text_operand operand) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  switch (operand.type) {
  case SQL_OPERAND_TYPE_COLUMN: {
    if (cJSON_AddStringToObject(result, "column", operand.value.column) ==
        NULL) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_OPERAND_TYPE_LITERAL: {
    const struct sql_literal literal = {.type = SQL_DATA_TYPE_TEXT,
                                        .value.text = operand.value.literal};
    cJSON *literal_json = serialize_literal(literal);
    if (literal_json == NULL ||
        !cJSON_AddItemToObject(result, "literal", literal_json)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  }

  return result;
}

static bool deserialize_text_operand(struct sql_text_operand *operand,
                                     const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *columnJSON = cJSON_GetObjectItem(json, "column");
  const cJSON *literalJSON = cJSON_GetObjectItem(json, "literal");
  if (columnJSON != NULL && literalJSON != NULL)
    return false;

  if (columnJSON != NULL) {
    if (!cJSON_IsString(columnJSON))
      return false;
    operand->type = SQL_OPERAND_TYPE_COLUMN;
    operand->value.column = strdup(columnJSON->valuestring);
  } else if (literalJSON != NULL) {
    struct sql_literal literal;
    if (!deserialize_literal(&literal, literalJSON))
      return false;
    operand->type = SQL_OPERAND_TYPE_LITERAL;
    operand->value.literal = literal.value.text;
  } else {
    return false;
  }

  return true;
}

static cJSON *serialize_comparison(struct sql_comparison comparison) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  const char *operator= comparison_operator_to_string[comparison.operator];
  if (cJSON_AddStringToObject(result, "operator", operator) == NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *left = serialize_operand(comparison.left);
  if (left == NULL || !cJSON_AddItemToObject(result, "left", left)) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *right = serialize_operand(comparison.right);
  if (right == NULL || !cJSON_AddItemToObject(result, "right", right)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_comparison(struct sql_comparison *comparison,
                                   const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *operatorJSON = cJSON_GetObjectItem(json, "operator");
  const cJSON *leftJSON = cJSON_GetObjectItem(json, "left");
  const cJSON *rightJSON = cJSON_GetObjectItem(json, "right");
  if (operatorJSON == NULL || leftJSON == NULL || rightJSON == NULL)
    return false;

  if (!cJSON_IsString(operatorJSON) ||
      !comparison_operator_from_string(&comparison->operator,
                                       operatorJSON->valuestring))
    return false;

  if (!deserialize_operand(&comparison->left, leftJSON) ||
      !deserialize_operand(&comparison->right, rightJSON))
    return false;

  return true;
}

static cJSON *serialize_contains(struct sql_contains contains) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  cJSON *left = serialize_text_operand(contains.left);
  if (left == NULL || !cJSON_AddItemToObject(result, "left", left)) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *right = serialize_text_operand(contains.right);
  if (right == NULL || !cJSON_AddItemToObject(result, "right", right)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_contains(struct sql_contains *contains,
                                 const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *leftJSON = cJSON_GetObjectItem(json, "left");
  const cJSON *rightJSON = cJSON_GetObjectItem(json, "right");
  if (leftJSON == NULL || rightJSON == NULL)
    return false;

  if (!deserialize_text_operand(&contains->left, leftJSON) ||
      !deserialize_text_operand(&contains->right, rightJSON))
    return false;

  return true;
}

static cJSON *serialize_filter(struct sql_filter filter) {
  if (filter.type == SQL_FILTER_TYPE_ALL) {
    return cJSON_CreateNull();
  }

  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  switch (filter.type) {
  case SQL_FILTER_TYPE_COMPARISON: {
    cJSON *comparison = serialize_comparison(filter.value.comparison);
    if (comparison == NULL ||
        !cJSON_AddItemToObject(result, "comparison", comparison)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_FILTER_TYPE_CONTAINS: {
    cJSON *contains = serialize_contains(filter.value.contains);
    if (contains == NULL ||
        !cJSON_AddItemToObject(result, "contains", contains)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_FILTER_TYPE_LOGIC: {
    cJSON *logic = cJSON_AddObjectToObject(result, "logic");
    const char *operator= logic_operator_to_string[filter.value.logic.operator];
    cJSON *left = serialize_filter(*filter.value.logic.left);
    cJSON *right = serialize_filter(*filter.value.logic.left);
    if (logic == NULL || left == NULL || right == NULL ||
        cJSON_AddStringToObject(logic, "operator", operator) == NULL ||
        !cJSON_AddItemToObject(logic, "left", left) ||
        !cJSON_AddItemToObject(logic, "right", right)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  default:
    break;
  }

  return result;
}

static bool deserialize_filter(struct sql_filter *filter, const cJSON *json) {
  if (json == NULL || cJSON_IsNull(json)) {
    filter->type = SQL_FILTER_TYPE_ALL;
    return true;
  }

  if (!cJSON_IsObject(json))
    return false;

  const cJSON *comparisonJSON = cJSON_GetObjectItem(json, "comparison");
  const cJSON *containsJSON = cJSON_GetObjectItem(json, "contains");
  const cJSON *logicJSON = cJSON_GetObjectItem(json, "logic");
  if (comparisonJSON != NULL && containsJSON != NULL)
    return false;
  if (comparisonJSON != NULL && logicJSON != NULL)
    return false;
  if (containsJSON != NULL && logicJSON != NULL)
    return false;

  if (comparisonJSON != NULL) {
    filter->type = SQL_FILTER_TYPE_COMPARISON;
    return deserialize_comparison(&filter->value.comparison, comparisonJSON);
  } else if (containsJSON != NULL) {
    filter->type = SQL_FILTER_TYPE_CONTAINS;
    return deserialize_contains(&filter->value.contains, containsJSON);
  } else if (logicJSON != NULL) {
    filter->type = SQL_FILTER_TYPE_LOGIC;
    struct sql_filter left, right;
    const cJSON *operatorJSON = cJSON_GetObjectItem(logicJSON, "operator");
    if (operatorJSON == NULL || !cJSON_IsString(operatorJSON) ||
        !deserialize_filter(&left, cJSON_GetObjectItem(logicJSON, "left")) ||
        !deserialize_filter(&right, cJSON_GetObjectItem(logicJSON, "right")))
      return false;
    logic_operator_from_string(&filter->value.logic.operator,
                               operatorJSON->valuestring);
    filter->value.logic.left = malloc(sizeof(struct sql_filter));
    filter->value.logic.right = malloc(sizeof(struct sql_filter));
    if (filter->value.logic.left == NULL || filter->value.logic.right == NULL) {
      free(filter->value.logic.left);
      free(filter->value.logic.right);
      return false;
    }
  } else {
    return false;
  }

  return true;
}

static cJSON *
serialize_column_with_literal(struct sql_column_with_literal column) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "name", column.name) == NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *literal = serialize_literal(column.literal);
  if (literal == NULL || !cJSON_AddItemToObject(result, "literal", literal)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool
deserialize_column_with_literal(struct sql_column_with_literal *column,
                                const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;
  const cJSON *nameJSON = cJSON_GetObjectItem(json, "name");
  const cJSON *literalJSON = cJSON_GetObjectItem(json, "literal");
  if (nameJSON == NULL || literalJSON == NULL)
    return false;

  if (!cJSON_IsString(nameJSON) ||
      !deserialize_literal(&column->literal, literalJSON))
    return false;

  column->name = strdup(nameJSON->valuestring);

  return true;
}

static cJSON *
serialize_column_with_literal_list(struct sql_column_with_literal_list *list) {
  cJSON *result = cJSON_CreateArray();
  if (result == NULL)
    return NULL;

  for (struct sql_column_with_literal_list *item = list; item != NULL;
       item = item->next) {
    cJSON *item_json = serialize_column_with_literal(item->item);
    if (item_json == NULL || !cJSON_AddItemToArray(result, item_json)) {
      cJSON_Delete(result);
      return NULL;
    }
  }

  return result;
}

static bool
deserialize_column_with_literal_list(struct sql_column_with_literal_list **list,
                                     const cJSON *json) {
  if (!cJSON_IsArray(json))
    return false;

  int count = cJSON_GetArraySize(json);
  *list = NULL;
  for (int i = 0; i < count; i++) {
    cJSON *item_json = cJSON_GetArrayItem(json, i);
    struct sql_column_with_literal item;
    if (item_json == NULL || !cJSON_IsObject(item_json) ||
        !deserialize_column_with_literal(&item, item_json)) {
      if (*list != NULL)
        sql_column_with_literal_list_free(*list);
      return false;
    }

    *list = sql_column_with_literal_list_create(item, *list);
  }

  return true;
}

static cJSON *serialize_join(struct sql_join join) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "join_table", join.join_table) == NULL ||
      cJSON_AddStringToObject(result, "table_column", join.table_column) ==
          NULL ||
      cJSON_AddStringToObject(result, "join_table_column",
                              join.join_table_column) == NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_join(struct sql_join *join, const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *join_tableJSON = cJSON_GetObjectItem(json, "join_table");
  const cJSON *table_columnJSON = cJSON_GetObjectItem(json, "table_column");
  const cJSON *join_table_columnJSON =
      cJSON_GetObjectItem(json, "join_table_column");
  if (join_tableJSON == NULL || table_columnJSON == NULL ||
      join_table_columnJSON == NULL)
    return false;
  if (!cJSON_IsString(join_tableJSON) || !cJSON_IsString(table_columnJSON) ||
      !cJSON_IsString(join_table_columnJSON))
    return false;

  join->join_table = strdup(join_tableJSON->valuestring);
  join->table_column = strdup(table_columnJSON->valuestring);
  join->join_table_column = strdup(join_table_columnJSON->valuestring);
  return true;
}

static cJSON *
serialize_create_statement(struct sql_create_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *columns = serialize_column_with_type_list(statement.columns);
  if (columns == NULL || !cJSON_AddItemToObject(result, "columns", columns)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_create_statement(struct sql_create_statement *statement,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;
  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  const cJSON *columnsJSON = cJSON_GetObjectItem(json, "columns");
  if (table_nameJSON == NULL || columnsJSON == NULL ||
      !cJSON_IsString(table_nameJSON) || !cJSON_IsArray(columnsJSON) ||
      !deserialize_column_with_type_list(&statement->columns, columnsJSON))
    return false;

  statement->table_name = strdup(table_nameJSON->valuestring);
  return true;
}

static cJSON *serialize_drop_statement(struct sql_drop_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_drop_statement(struct sql_drop_statement *statement,
                                       const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  if (table_nameJSON == NULL || !cJSON_IsString(table_nameJSON))
    return false;

  statement->table_name = strdup(table_nameJSON->valuestring);
  return true;
}

static cJSON *
serialize_select_statement(struct sql_select_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *filter = serialize_filter(statement.filter);
  if (filter == NULL || !cJSON_AddItemToObject(result, "filter", filter)) {
    cJSON_Delete(result);
    return NULL;
  }

  if (statement.join.has_value) {
    cJSON *join = serialize_join(statement.join.value);
    if (join == NULL || !cJSON_AddItemToObject(result, "join", join)) {
      cJSON_Delete(result);
      return NULL;
    }
  }

  return result;
}

static bool deserialize_select_statement(struct sql_select_statement *statement,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;
  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  const cJSON *filterJSON = cJSON_GetObjectItem(json, "filter");
  const cJSON *joinJSON = cJSON_GetObjectItem(json, "join");
  if (table_nameJSON == NULL || filterJSON == NULL)
    return false;

  if (!cJSON_IsString(table_nameJSON) ||
      !deserialize_filter(&statement->filter, filterJSON))
    return false;
  statement->table_name = strdup(table_nameJSON->valuestring);

  if (joinJSON == NULL || !deserialize_join(&statement->join.value, joinJSON)) {
    statement->join.has_value = false;
    return true;
  }

  statement->join.has_value = true;
  return true;
}

static cJSON *
serialize_insert_statement(struct sql_insert_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *values = serialize_literal_list(statement.values);
  if (values == NULL || !cJSON_AddItemToObject(result, "values", values)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_insert_statement(struct sql_insert_statement *statement,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  const cJSON *valuesJSON = cJSON_GetObjectItem(json, "values");
  if (table_nameJSON == NULL || valuesJSON == NULL ||
      !cJSON_IsString(table_nameJSON) || !cJSON_IsArray(valuesJSON) ||
      !deserialize_literal_list(&statement->values, valuesJSON))
    return false;

  statement->table_name = strdup(table_nameJSON->valuestring);
  return true;
}

static cJSON *
serialize_delete_statement(struct sql_delete_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *filter = serialize_filter(statement.filter);
  if (filter == NULL || !cJSON_AddItemToObject(result, "filter", filter)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_delete_statement(struct sql_delete_statement *statement,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  const cJSON *filterJSON = cJSON_GetObjectItem(json, "filter");
  if (table_nameJSON == NULL || filterJSON == NULL)
    return false;

  if (!cJSON_IsString(table_nameJSON) ||
      !deserialize_filter(&statement->filter, filterJSON))
    return false;

  statement->table_name = strdup(table_nameJSON->valuestring);
  return true;
}

static cJSON *
serialize_update_statement(struct sql_update_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  if (cJSON_AddStringToObject(result, "table_name", statement.table_name) ==
      NULL) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *filter = serialize_filter(statement.filter);
  if (filter == NULL || !cJSON_AddItemToObject(result, "filter", filter)) {
    cJSON_Delete(result);
    return NULL;
  }

  cJSON *set = serialize_column_with_literal_list(statement.set);
  if (set == NULL || !cJSON_AddItemToObject(result, "set", set)) {
    cJSON_Delete(result);
    return NULL;
  }

  return result;
}

static bool deserialize_update_statement(struct sql_update_statement *statement,
                                         const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *table_nameJSON = cJSON_GetObjectItem(json, "table_name");
  const cJSON *filterJSON = cJSON_GetObjectItem(json, "filter");
  const cJSON *setJSON = cJSON_GetObjectItem(json, "set");
  if (table_nameJSON == NULL || filterJSON == NULL || setJSON == NULL)
    return false;

  if (!cJSON_IsString(table_nameJSON) ||
      !deserialize_filter(&statement->filter, filterJSON) ||
      !deserialize_column_with_literal_list(&statement->set, setJSON))
    return false;

  statement->table_name = strdup(table_nameJSON->valuestring);
  return true;
}

static cJSON *serialize_statement(struct sql_statement statement) {
  cJSON *result = cJSON_CreateObject();
  if (result == NULL)
    return NULL;

  switch (statement.type) {
  case SQL_STATEMENT_TYPE_CREATE: {
    cJSON *create = serialize_create_statement(statement.value.create);
    if (create == NULL || !cJSON_AddItemToObject(result, "create", create)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_STATEMENT_TYPE_DROP: {
    cJSON *drop = serialize_drop_statement(statement.value.drop);
    if (drop == NULL || !cJSON_AddItemToObject(result, "drop", drop)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_STATEMENT_TYPE_INSERT: {
    cJSON *insert = serialize_insert_statement(statement.value.insert);
    if (insert == NULL || !cJSON_AddItemToObject(result, "insert", insert)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_STATEMENT_TYPE_SELECT: {
    cJSON *select = serialize_select_statement(statement.value.select);
    if (select == NULL || !cJSON_AddItemToObject(result, "select", select)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_STATEMENT_TYPE_DELETE: {
    cJSON *delete = serialize_delete_statement(statement.value.delete);
    if (delete == NULL || !cJSON_AddItemToObject(result, "delete", delete)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  case SQL_STATEMENT_TYPE_UPDATE: {
    cJSON *update = serialize_update_statement(statement.value.update);
    if (update == NULL || !cJSON_AddItemToObject(result, "update", update)) {
      cJSON_Delete(result);
      return NULL;
    }
  } break;
  }

  return result;
}

static bool deserialize_statement(struct sql_statement *statement,
                                  const cJSON *json) {
  if (!cJSON_IsObject(json))
    return false;

  const cJSON *createJSON = cJSON_GetObjectItem(json, "create");
  const cJSON *dropJSON = cJSON_GetObjectItem(json, "drop");
  const cJSON *insertJSON = cJSON_GetObjectItem(json, "insert");
  const cJSON *selectJSON = cJSON_GetObjectItem(json, "select");
  const cJSON *deleteJSON = cJSON_GetObjectItem(json, "delete");
  const cJSON *updateJSON = cJSON_GetObjectItem(json, "update");
  if (createJSON == NULL && dropJSON == NULL && insertJSON == NULL &&
      selectJSON == NULL && deleteJSON == NULL && updateJSON == NULL)
    return false;

  if (createJSON != NULL) {
    if (!deserialize_create_statement(&statement->value.create, createJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_CREATE;
  }
  if (dropJSON != NULL) {
    if (!deserialize_drop_statement(&statement->value.drop, dropJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_DROP;
  }
  if (insertJSON != NULL) {
    if (!deserialize_insert_statement(&statement->value.insert, insertJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_INSERT;
  }
  if (selectJSON != NULL) {
    if (!deserialize_select_statement(&statement->value.select, selectJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_SELECT;
  }
  if (deleteJSON != NULL) {
    if (!deserialize_delete_statement(&statement->value.delete, deleteJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_DELETE;
  }
  if (updateJSON != NULL) {
    if (!deserialize_update_statement(&statement->value.update, updateJSON))
      return false;
    statement->type = SQL_STATEMENT_TYPE_UPDATE;
  }

  return true;
}

char *serialize(struct sql_statement statement) {
  cJSON *json = serialize_statement(statement);
  if (json == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  char *string;
#ifndef NDEBUG
  string = cJSON_Print(json);
#else
  string = cJSON_PrintUnformatted(json);
#endif

  cJSON_Delete(json);
  return string;
}

struct models_deserialization_result deserialize(const char *string) {
  cJSON *json = cJSON_Parse(string);
  if (json == NULL)
    return (struct models_deserialization_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};

  struct models_deserialization_result result;
  result.type = deserialize_statement(&result.value, json)
                    ? MODELS_DESERIALIZATION_RESULT_OK
                    : MODELS_DESERIALIZATION_RESULT_ERROR;

  cJSON_Delete(json);
  return result;
}

char *serialize_common_response(struct sql_common_response response) {
  cJSON *json = cJSON_CreateObject();
  if (json == NULL)
    return NULL;

  if (cJSON_AddStringToObject(json, "message", response.message) == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  char *string;
#ifndef NDEBUG
  string = cJSON_Print(json);
#else
  string = cJSON_PrintUnformatted(json);
#endif

  cJSON_Delete(json);
  return string;
}

struct models_deserialization_common_response_result
deserialize_common_response(const char *string) {
  cJSON *json = cJSON_Parse(string);
  if (json == NULL)
    return (struct models_deserialization_common_response_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};

  cJSON *messageJSON = cJSON_GetObjectItem(json, "message");
  if (messageJSON == NULL || !cJSON_IsString(messageJSON)) {
    cJSON_Delete(json);
    return (struct models_deserialization_common_response_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};
  }

  cJSON_Delete(json);
  return (struct models_deserialization_common_response_result){
      .type = MODELS_DESERIALIZATION_RESULT_OK,
      .value.message = messageJSON->valuestring};
}

struct models_deserialization_select_response_result
deserialize_select_response(const char *string) {
  cJSON *json = cJSON_Parse(string);
  if (json == NULL)
    return (struct models_deserialization_select_response_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};

  cJSON *columnsJSON = cJSON_GetObjectItem(json, "columns");
  if (columnsJSON == NULL || !cJSON_IsArray(columnsJSON)) {
    cJSON_Delete(json);
    return (struct models_deserialization_select_response_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};
  }

  size_t columns_count = cJSON_GetArraySize(columnsJSON);
  char **columns = malloc(sizeof(char *) * columns_count);
  for (int i = 0; i < columns_count; i++) {
    cJSON *columnJSON = cJSON_GetArrayItem(columnsJSON, i);
    if (columnJSON == NULL || !cJSON_IsString(columnJSON)) {
      free(columns);
      cJSON_Delete(json);
      return (struct models_deserialization_select_response_result){
          .type = MODELS_DESERIALIZATION_RESULT_ERROR};
    }
    columns[i] = strdup(columnJSON->valuestring);
  }

  struct sql_select_response response;
  response.header.columns_count = columns_count;
  response.header.columns = columns;

  cJSON *rowsJSON = cJSON_GetObjectItem(json, "rows");
  if (rowsJSON == NULL ||
      !deserialize_literal_list_list(&response.rows, rowsJSON)) {
    free(columns);
    cJSON_Delete(json);
    return (struct models_deserialization_select_response_result){
        .type = MODELS_DESERIALIZATION_RESULT_ERROR};
  }

  cJSON_Delete(json);
  return (struct models_deserialization_select_response_result){
      .type = MODELS_DESERIALIZATION_RESULT_OK, .value = response};
}

char *serialize_select_response(struct sql_select_response response) {
  cJSON *json = cJSON_CreateObject();
  if (json == NULL)
    return NULL;

  cJSON *rowsJSON = serialize_literal_list_list(response.rows);
  if (rowsJSON == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  if (!cJSON_AddItemToObject(json, "rows", rowsJSON)) {
    cJSON_Delete(json);
    return NULL;
  }

  cJSON *columnsJSON = cJSON_AddArrayToObject(json, "columns");
  if (columnsJSON == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  for (size_t i = 0; i < response.header.columns_count; i++) {
    cJSON *columnJSON = cJSON_CreateString(response.header.columns[i]);
    if (columnJSON == NULL) {
      cJSON_Delete(json);
      return NULL;
    }
    if (!cJSON_AddItemToArray(columnsJSON, columnJSON)) {
      cJSON_Delete(json);
      return NULL;
    }
  }

  char *string;
#ifndef NDEBUG
  string = cJSON_Print(json);
#else
  string = cJSON_PrintUnformatted(json);
#endif

  cJSON_Delete(json);
  return string;
}
