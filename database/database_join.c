#include "database_join.h"
#include <string.h>

bool database_join_is_satisfied(struct database_table left_table,
                                struct database_row left_row,
                                struct database_table right_table,
                                struct database_row right_row,
                                struct database_join join) {
  const struct database_attribute left_attribute = database_attributes_get(
      left_table.attributes, join.left_attribute_position);
  const struct database_attribute right_attribute = database_attributes_get(
      right_table.attributes, join.right_attribute_position);
  if (left_attribute.type != right_attribute.type) {
    return false;
  }

  const union database_attribute_value left_value =
      database_attribute_values_get(left_row.values,
                                    join.left_attribute_position);
  const union database_attribute_value right_value =
      database_attribute_values_get(right_row.values,
                                    join.right_attribute_position);
  switch (left_attribute.type) {
  case DATABASE_ATTRIBUTE_INTEGER:
    return left_value.integer == right_value.integer;
  case DATABASE_ATTRIBUTE_FLOATING_POINT:
    return left_value.floating_point == right_value.floating_point;
  case DATABASE_ATTRIBUTE_BOOLEAN:
    return left_value.boolean == right_value.boolean;
  case DATABASE_ATTRIBUTE_STRING:
    return strcmp(left_value.string, right_value.string) == 0;
  default:
    return false;
  }
}