#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_H

#include "database_attribute_type.h"

struct database_attribute {
  char *name;
  enum database_attribute_type type;
};

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_H
