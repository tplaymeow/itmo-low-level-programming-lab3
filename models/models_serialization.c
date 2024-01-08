#include "models_serialization.h"
#include "cjson/cJSON.h"

const char *data_type_to_string[] = {
    [SQL_DATA_TYPE_INTEGER] = "integer",
    [SQL_DATA_TYPE_FLOATING_POINT] = "floating_point",
    [SQL_DATA_TYPE_BOOLEAN] = "boolean",
    [SQL_DATA_TYPE_TEXT] = "text",
};

const char *comparison_operator_to_string[] = {
    [SQL_COMPARISON_OPERATOR_EQUAL] = "EQUAL",
    [SQL_COMPARISON_OPERATOR_NOT_EQUAL] = "NOT_EQUAL",
    [SQL_COMPARISON_OPERATOR_GREATER] = "GREATER",
    [SQL_COMPARISON_OPERATOR_GREATER_OR_EQUAL] = "GREATER_OR_EQUAL",
    [SQL_COMPARISON_OPERATOR_LESS] = "LESS",
    [SQL_COMPARISON_OPERATOR_LESS_OR_EQUAL] = "LESS_OR_EQUAL",
};

const char *logic_operator_to_string[] = {
    [SQL_LOGIC_BINARY_OPERATOR_AND] = "AND",
    [SQL_LOGIC_BINARY_OPERATOR_OR] = "OR",
};

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

struct models_deserialization_result deserialize(char *string) {}
