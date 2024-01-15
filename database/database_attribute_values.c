#include "database_attribute_values.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct database_attribute_values
database_attribute_values_create(size_t count) {
  union database_attribute_value *values =
      malloc(count * sizeof(union database_attribute_value));
  return (struct database_attribute_values){.count = count, .values = values};
}

void database_attribute_values_destroy(
    struct database_attribute_values values) {
  if (values.values) {
    free(values.values);
  }
}

union database_attribute_value
database_attribute_values_get(struct database_attribute_values values,
                              size_t position) {
  assert(position < values.count);
  return values.values[position];
}

void database_attribute_values_set(struct database_attribute_values values,
                                   size_t position,
                                   union database_attribute_value value) {
  if (position < values.count) {
    values.values[position] = value;
  } else {
    assert(false);
  }
}
