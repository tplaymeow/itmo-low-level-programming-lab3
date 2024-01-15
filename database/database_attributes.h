#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTES_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTES_H

#include <stddef.h>

#include "database_attribute.h"

struct database_attributes {
  size_t count;
  struct database_attribute *values;
};

struct database_attributes database_attributes_create(size_t count);

void database_attributes_destroy(struct database_attributes value);

struct database_attribute
database_attributes_get(struct database_attributes attributes, size_t position);

void database_attributes_set(struct database_attributes attributes,
                             size_t position, struct database_attribute value);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTES_H
