#include "models.h"
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