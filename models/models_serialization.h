#ifndef LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H
#define LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H

#include "models.h"

char *serialize(struct sql_statement statement);

char *serialize_common_response(struct sql_common_response response);

char *serialize_select_response(struct sql_select_response response);

enum models_deserialization_result_type {
  MODELS_DESERIALIZATION_RESULT_OK,
  MODELS_DESERIALIZATION_RESULT_ERROR
};

struct models_deserialization_result {
  enum models_deserialization_result_type type;
  struct sql_statement value;
};

struct models_deserialization_common_response_result {
  enum models_deserialization_result_type type;
  struct sql_common_response value;
};

struct models_deserialization_select_response_result {
  enum models_deserialization_result_type type;
  struct sql_select_response value;
};

struct models_deserialization_result deserialize(const char *string);

struct models_deserialization_common_response_result
deserialize_common_response(const char *string);

struct models_deserialization_select_response_result
deserialize_select_response(const char *string);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_MODELS_SERIALIZATION_H
