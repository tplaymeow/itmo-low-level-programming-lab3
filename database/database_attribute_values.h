#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUES_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUES_H

#include <stddef.h>

#include "database_attribute_value.h"

struct database_attribute_values {
  size_t count;
  union database_attribute_value *values;
};

struct database_attribute_values database_attribute_values_create(size_t count);

void database_attribute_values_destroy(struct database_attribute_values values);

union database_attribute_value
database_attribute_values_get(struct database_attribute_values values,
                              size_t position);

void database_attribute_values_set(struct database_attribute_values values,
                                   size_t position,
                                   union database_attribute_value value);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUES_H
