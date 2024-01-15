#include "database_insert_row_request.h"
#include "database_attribute_value.h"
#include "database_table.h"
#include <stdlib.h>

struct database_insert_row_request
database_insert_row_request_create(struct database_table table) {
  struct database_attribute_values values =
      database_attribute_values_create(table.attributes.count);
  return (struct database_insert_row_request){.values = values};
}

void database_insert_row_request_destroy(
    struct database_insert_row_request request) {
  database_attribute_values_destroy(request.values);
}

union database_attribute_value database_insert_row_request_get_value(
    struct database_insert_row_request request, size_t position) {
  return database_attribute_values_get(request.values, position);
}

void database_insert_row_request_set_value(
    struct database_insert_row_request request, size_t position,
    union database_attribute_value value) {
  database_attribute_values_set(request.values, position, value);
}