#ifndef LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H
#define LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H

#include "models.h"

struct parsing_result {
  enum { PARSING_STATUS_OK, PARSING_STATUS_ERROR } status;
  struct sql_statement value;
};

struct parsing_result parse(const char *input);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H
