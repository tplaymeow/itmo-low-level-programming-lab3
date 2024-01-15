#include "database_create_table_request.h"
#include "database_attribute.h"
#include "database_attributes.h"
#include <stdlib.h>

struct database_create_table_request
database_create_table_request_create(const char *table_name,
                                     size_t attributes_count) {
  struct database_attributes attributes =
      database_attributes_create(attributes_count);
  return (struct database_create_table_request){.name = table_name,
                                                .attributes = attributes};
}

void database_create_table_request_destroy(
    struct database_create_table_request request) {
  database_attributes_destroy(request.attributes);
}

struct database_attribute database_create_table_request_get_attribute(
    struct database_create_table_request request, size_t position) {
  return database_attributes_get(request.attributes, position);
}

void database_create_table_request_set_attribute(
    struct database_create_table_request request, size_t position,
    struct database_attribute attribute) {
  database_attributes_set(request.attributes, position, attribute);
}
