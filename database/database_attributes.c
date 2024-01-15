#include "database_attributes.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct database_attributes database_attributes_create(size_t count) {
  struct database_attribute *values =
      malloc(count * sizeof(struct database_attribute));
  return (struct database_attributes){.count = count, .values = values};
}

void database_attributes_destroy(struct database_attributes value) {
  if (value.values) {
    free(value.values);
  }
}

struct database_attribute
database_attributes_get(struct database_attributes attributes,
                        size_t position) {
  assert(position < attributes.count);
  return attributes.values[position];
}

void database_attributes_set(struct database_attributes attributes,
                             size_t position, struct database_attribute value) {
  if (position < attributes.count) {
    attributes.values[position] = value;
  } else {
    assert(false);
  }
}
