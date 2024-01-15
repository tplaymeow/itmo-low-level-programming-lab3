#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUE_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUE_H

#include <stdbool.h>
#include <stdint.h>

union database_attribute_value {
  int64_t integer;
  double floating_point;
  bool boolean;
  char *string;
};

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_DATABASE_ATTRIBUTE_VALUE_H
