#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_TABLE_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_TABLE_H

#include "database_attribute.h"
#include "database_attributes.h"
#include "paging.h"
// #include "pa"

struct database_table {
  void *data;
  char *name;
  struct paging_info page_info;
  struct database_attributes attributes;
};

void database_table_destroy(struct database_table table);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_TABLE_H
