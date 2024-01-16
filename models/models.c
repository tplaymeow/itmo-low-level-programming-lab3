#include "models.h"
#include <assert.h>
#include <stdlib.h>

struct sql_column_with_type_list *
sql_column_with_type_list_create(struct sql_column_with_type item,
                                 struct sql_column_with_type_list *next) {
  struct sql_column_with_type_list *result =
      malloc(sizeof(struct sql_column_with_type_list));
  if (result == NULL)
    return NULL;

  result->item = item;
  result->next = next;
  return result;
}

void sql_column_with_type_list_free(struct sql_column_with_type_list *list) {
  if (list == NULL)
    return;

  sql_column_with_type_list_free(list->next);
  free(list);
}

struct sql_literal_list *
sql_literal_list_create(struct sql_literal item,
                        struct sql_literal_list *next) {
  struct sql_literal_list *result = malloc(sizeof(struct sql_literal_list));
  if (result == NULL)
    return NULL;

  result->item = item;
  result->next = next;
  return result;
}

void sql_literal_list_free(struct sql_literal_list *list) {
  if (list == NULL)
    return;

  sql_literal_list_free(list->next);
  free(list);
}

struct sql_literal_list_list *
sql_literal_list_list_create(struct sql_literal_list *item,
                             struct sql_literal_list_list *next) {
  struct sql_literal_list_list *result =
      malloc(sizeof(struct sql_literal_list_list));
  if (result == NULL)
    return NULL;

  result->item = item;
  result->next = next;
  return result;
}

void sql_literal_list_list_free(struct sql_literal_list_list *list) {
  if (list == NULL)
    return;

  sql_literal_list_free(list->item);
  sql_literal_list_list_free(list->next);
  free(list);
}

struct sql_column_with_literal_list *
sql_column_with_literal_list_create(struct sql_column_with_literal item,
                                    struct sql_column_with_literal_list *next) {
  struct sql_column_with_literal_list *result =
      malloc(sizeof(struct sql_column_with_literal_list));
  if (result == NULL)
    return NULL;

  result->item = item;
  result->next = next;
  return result;
}

void sql_column_with_literal_list_free(
    struct sql_column_with_literal_list *list) {
  if (list == NULL)
    return;

  sql_column_with_literal_list_free(list->next);
  free(list);
}

struct sql_select_response_header
sql_select_response_header_create(size_t columns_count) {
  return (struct sql_select_response_header){
      .columns_count = columns_count,
      .columns = malloc(columns_count * sizeof(char *))};
}

void sql_select_response_header_destroy(
    struct sql_select_response_header header) {
  if (header.columns != NULL) {
    free(header.columns);
  } else {
    assert("");
  }
}
