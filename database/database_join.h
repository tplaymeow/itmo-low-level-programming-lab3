#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_JOIN_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_JOIN_H

#include "database_row.h"
#include "database_table.h"
#include <stdbool.h>
#include <stddef.h>

struct database_join {
  size_t left_attribute_position;
  size_t right_attribute_position;
};

bool database_join_is_satisfied(struct database_table left_table,
                                struct database_row left_row,
                                struct database_table right_table,
                                struct database_row right_row,
                                struct database_join join);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_JOIN_H
