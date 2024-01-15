#ifndef LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H
#define LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H

#include "models.h"

struct parsing_result {
  enum { PARSING_STATUS_OK, PARSING_STATUS_ERROR } status;
  struct parsing_result_value {
    enum { PARSING_TYPE_STATEMENT, PARSING_TYPE_EXIT } type;
    union {
      struct sql_statement statement;
    } value;
  } value;
};

struct parsing_result parse(const char *input);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_PARSING_H
