#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ROW_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ROW_H

#include "database_attribute_values.h"
#include "paging.h"

struct database_row {
  void *data;
  struct paging_info paging_info;
  struct database_attribute_values values;
};

void database_row_destroy(struct database_row row);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ROW_H
