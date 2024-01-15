#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_CREATE_TABLE_REQUEST_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_CREATE_TABLE_REQUEST_H

#include <stddef.h>

#include "database_attribute.h"
#include "database_attributes.h"

struct database_create_table_request {
  const char *name;
  struct database_attributes attributes;
};

struct database_create_table_request
database_create_table_request_create(const char *table_name,
                                     size_t attributes_count);

void database_create_table_request_destroy(
    struct database_create_table_request request);

struct database_attribute database_create_table_request_get_attribute(
    struct database_create_table_request request, size_t position);

void database_create_table_request_set_attribute(
    struct database_create_table_request request, size_t position,
    struct database_attribute attribute);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_CREATE_TABLE_REQUEST_H
