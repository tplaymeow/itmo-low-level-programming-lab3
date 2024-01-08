#ifndef LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H
#define LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H

#include "models.h"

char *serialize(struct sql_statement statement);

struct models_deserialization_result {
  enum {
    MODELS_DESERIALIZATION_RESULT_OK,
    MODELS_DESERIALIZATION_RESULT_ERROR
  } type;
  struct sql_statement value;
};

struct models_deserialization_result deserialize(char *string);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H
