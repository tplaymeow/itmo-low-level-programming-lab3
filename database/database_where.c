#include "database_where.h"
#include "database_attribute.h"
#include "database_attribute_value.h"
#include <stdlib.h>
#include <string.h>

static bool database_where_joined_is_satisfied_contains(
    struct database_table left_table, struct database_table right_table,
    struct database_row left_row, struct database_row right_row,
    struct database_where_joined_contains contains) {
  const char *left_value;
  const char *right_value;
  switch (contains.left.type) {
  case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
    left_value = contains.left.value.constant.value;
    switch (contains.right.type) {
    case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
      right_value = contains.right.value.constant.value;
    } break;

    case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
      struct database_table table;
      struct database_row row;
      if (contains.right.value.attribute.table_position == 0) {
        table = left_table;
        row = left_row;
      } else if (contains.right.value.attribute.table_position == 1) {
        table = right_table;
        row = right_row;
      } else {
        return false;
      }

      const size_t right_position =
          contains.right.value.attribute.attribute_position;
      if (database_attributes_get(table.attributes, right_position).type !=
          DATABASE_ATTRIBUTE_STRING) {
        return false;
      }

      right_value =
          database_attribute_values_get(row.values, right_position).string;
    } break;
    }
  } break;
  case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
    struct database_table l_item_table;
    struct database_row l_item_row;
    if (contains.left.value.attribute.table_position == 0) {
      l_item_table = left_table;
      l_item_row = left_row;
    } else if (contains.left.value.attribute.table_position == 1) {
      l_item_table = right_table;
      l_item_row = right_row;
    } else {
      return false;
    }

    const size_t left_position =
        contains.left.value.attribute.attribute_position;
    if (database_attributes_get(l_item_table.attributes, left_position).type !=
        DATABASE_ATTRIBUTE_STRING) {
      return false;
    }

    left_value =
        database_attribute_values_get(l_item_row.values, left_position).string;

    switch (contains.right.type) {
    case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
      right_value = contains.right.value.constant.value;
    } break;

    case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
      struct database_table r_item_table;
      struct database_row r_item_row;
      if (contains.right.value.attribute.table_position == 0) {
        r_item_table = left_table;
        r_item_row = left_row;
      } else if (contains.right.value.attribute.table_position == 1) {
        r_item_table = right_table;
        r_item_row = right_row;
      } else {
        return false;
      }

      const size_t right_position =
          contains.right.value.attribute.attribute_position;
      if (database_attributes_get(r_item_table.attributes, right_position)
              .type != DATABASE_ATTRIBUTE_STRING) {
        return false;
      }

      right_value =
          database_attribute_values_get(r_item_row.values, right_position)
              .string;
    } break;
    }
  } break;
  }

  return strstr(left_value, right_value) != NULL;
}

static bool
database_where_is_satisfied_contains(struct database_table table,
                                     struct database_row row,
                                     struct database_where_contains contains) {
  const char *left_value;
  const char *right_value;
  switch (contains.left.type) {
  case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
    left_value = contains.left.value.constant.value;
    switch (contains.right.type) {
    case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
      right_value = contains.right.value.constant.value;
    } break;
    case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
      const size_t right_position =
          contains.right.value.attribute.attribute_position;
      if (database_attributes_get(table.attributes, right_position).type !=
          DATABASE_ATTRIBUTE_STRING) {
        return false;
      }
      right_value =
          database_attribute_values_get(row.values, right_position).string;
    } break;
    }
  } break;
  case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
    const size_t left_position =
        contains.left.value.attribute.attribute_position;
    if (database_attributes_get(table.attributes, left_position).type !=
        DATABASE_ATTRIBUTE_STRING) {
      return false;
    }
    left_value =
        database_attribute_values_get(row.values, left_position).string;
    switch (contains.right.type) {
    case DATABASE_WHERE_CONTAINS_ITEM_CONSTANT: {
      right_value = contains.right.value.constant.value;
    } break;
    case DATABASE_WHERE_CONTAINS_ITEM_ATTRIBUTE: {
      const size_t right_position =
          contains.right.value.attribute.attribute_position;
      if (database_attributes_get(table.attributes, right_position).type !=
          DATABASE_ATTRIBUTE_STRING) {
        return false;
      }

      right_value =
          database_attribute_values_get(row.values, right_position).string;
    } break;
    }
  } break;
  }

  return strstr(left_value, right_value) != NULL;
}

static bool database_where_is_satisfied_comparison(
    struct database_table table, struct database_row row,
    struct database_where_comparison comparison) {
  enum database_attribute_type type;
  union database_attribute_value left_value;
  union database_attribute_value right_value;
  switch (comparison.left.type) {
  case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
    type = comparison.left.data_type;
    left_value = comparison.left.value.constant.value;
    switch (comparison.right.type) {
    case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
      if (type != comparison.right.data_type) {
        return false;
      }
      right_value = comparison.right.value.constant.value;
    } break;

    case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
      const size_t right_position =
          comparison.right.value.attribute.attribute_position;
      if (type !=
          database_attributes_get(table.attributes, right_position).type) {
        return false;
      }
      right_value = database_attribute_values_get(row.values, right_position);
    } break;
    }
  } break;
  case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
    const size_t left_position =
        comparison.left.value.attribute.attribute_position;
    type = database_attributes_get(table.attributes, left_position).type;
    left_value = database_attribute_values_get(row.values, left_position);
    switch (comparison.right.type) {
    case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
      if (type != comparison.right.data_type) {
        return false;
      }
      right_value = comparison.right.value.constant.value;
    } break;
    case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
      const size_t right_position =
          comparison.right.value.attribute.attribute_position;
      if (type !=
          database_attributes_get(table.attributes, right_position).type) {
        return false;
      }
      right_value = database_attribute_values_get(row.values, right_position);
    } break;
    }
  } break;
  }

  bool is_less, is_greater;
  switch (type) {
  case DATABASE_ATTRIBUTE_INTEGER: {
    is_less = left_value.integer < right_value.integer;
    is_greater = left_value.integer > right_value.integer;
  } break;
  case DATABASE_ATTRIBUTE_FLOATING_POINT: {
    is_less = left_value.floating_point < right_value.floating_point;
    is_greater = left_value.floating_point > right_value.floating_point;
  } break;
  case DATABASE_ATTRIBUTE_BOOLEAN: {
    is_less = left_value.boolean < right_value.boolean;
    is_greater = left_value.boolean > right_value.boolean;
  } break;
  case DATABASE_ATTRIBUTE_STRING: {
    const int strcmp_res = strcmp(left_value.string, right_value.string);
    is_less = strcmp_res < 0;
    is_greater = strcmp_res > 0;
  } break;
  }

  switch (comparison.operator) {
  case DATABASE_WHERE_COMPARISON_OPERATOR_EQUAL:
    return !is_less && !is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_NOT_EQUAL:
    return is_less || is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_GREATER:
    return is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_GREATER_OR_EQUAL:
    return !is_less;
  case DATABASE_WHERE_COMPARISON_OPERATOR_LESS:
    return is_less;
  case DATABASE_WHERE_COMPARISON_OPERATOR_LESS_OR_EQUAL:
    return !is_greater;
  }
}

static bool database_where_joined_is_satisfied_comparison(
    struct database_table left_table, struct database_table right_table,
    struct database_row left_row, struct database_row right_row,
    struct database_where_joined_comparison comparison) {
  enum database_attribute_type type;
  union database_attribute_value left_value;
  union database_attribute_value right_value;
  switch (comparison.left.type) {
  case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
    type = comparison.left.data_type;
    left_value = comparison.left.value.constant.value;
    switch (comparison.right.type) {
    case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
      if (type != comparison.right.data_type) {
        return false;
      }
      right_value = comparison.right.value.constant.value;
    } break;

    case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
      struct database_table table;
      struct database_row row;
      if (comparison.right.value.attribute.table_position == 0) {
        table = left_table;
        row = left_row;
      } else if (comparison.right.value.attribute.table_position == 1) {
        table = right_table;
        row = right_row;
      } else {
        return false;
      }

      const size_t right_position =
          comparison.right.value.attribute.attribute_position;
      if (type !=
          database_attributes_get(table.attributes, right_position).type) {
        return false;
      }

      right_value = database_attribute_values_get(row.values, right_position);
    } break;
    }
  } break;
  case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
    struct database_table l_item_table;
    struct database_row l_item_row;
    if (comparison.left.value.attribute.table_position == 0) {
      l_item_table = left_table;
      l_item_row = left_row;
    } else if (comparison.left.value.attribute.table_position == 1) {
      l_item_table = right_table;
      l_item_row = right_row;
    } else {
      return false;
    }

    const size_t left_position =
        comparison.left.value.attribute.attribute_position;
    type = database_attributes_get(l_item_table.attributes, left_position).type;
    left_value =
        database_attribute_values_get(l_item_row.values, left_position);
    switch (comparison.right.type) {
    case DATABASE_WHERE_COMPARISON_ITEM_CONSTANT: {
      if (type != comparison.right.data_type) {
        return false;
      }
      right_value = comparison.right.value.constant.value;
    } break;
    case DATABASE_WHERE_COMPARISON_ITEM_ATTRIBUTE: {
      struct database_table r_item_table;
      struct database_row r_item_row;
      if (comparison.right.value.attribute.table_position == 0) {
        r_item_table = left_table;
        r_item_row = left_row;
      } else if (comparison.right.value.attribute.table_position == 1) {
        r_item_table = right_table;
        r_item_row = right_row;
      } else {
        return false;
      }

      const size_t right_position =
          comparison.right.value.attribute.attribute_position;
      if (type !=
          database_attributes_get(r_item_table.attributes, right_position)
              .type) {
        return false;
      }

      right_value =
          database_attribute_values_get(r_item_row.values, right_position);
    } break;
    }
  } break;
  }

  bool is_less, is_greater;
  switch (type) {
  case DATABASE_ATTRIBUTE_INTEGER: {
    is_less = left_value.integer < right_value.integer;
    is_greater = left_value.integer > right_value.integer;
  } break;
  case DATABASE_ATTRIBUTE_FLOATING_POINT: {
    is_less = left_value.floating_point < right_value.floating_point;
    is_greater = left_value.floating_point > right_value.floating_point;
  } break;
  case DATABASE_ATTRIBUTE_BOOLEAN: {
    is_less = left_value.boolean < right_value.boolean;
    is_greater = left_value.boolean > right_value.boolean;
  } break;
  case DATABASE_ATTRIBUTE_STRING: {
    const int strcmp_res = strcmp(left_value.string, right_value.string);
    is_less = strcmp_res < 0;
    is_greater = strcmp_res > 0;
  } break;
  }

  switch (comparison.operator) {
  case DATABASE_WHERE_COMPARISON_OPERATOR_EQUAL:
    return !is_less && !is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_NOT_EQUAL:
    return is_less || is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_GREATER:
    return is_greater;
  case DATABASE_WHERE_COMPARISON_OPERATOR_GREATER_OR_EQUAL:
    return !is_less;
  case DATABASE_WHERE_COMPARISON_OPERATOR_LESS:
    return is_less;
  case DATABASE_WHERE_COMPARISON_OPERATOR_LESS_OR_EQUAL:
    return !is_greater;
  }
}

bool database_where_is_satisfied(struct database_table table,
                                 struct database_row row,
                                 struct database_where where) {
  switch (where.type) {
  case DATABASE_WHERE_TYPE_ALWAYS:
    return true;

  case DATABASE_WHERE_TYPE_LOGIC:
    switch (where.value.logic.operator) {
    case DATABASE_WHERE_LOGIC_OPERATOR_AND:
      return database_where_is_satisfied(table, row, *where.value.logic.left) &&
             database_where_is_satisfied(table, row, *where.value.logic.right);

    case DATABASE_WHERE_LOGIC_OPERATOR_OR:
      return database_where_is_satisfied(table, row, *where.value.logic.left) ||
             database_where_is_satisfied(table, row, *where.value.logic.right);
    }

  case DATABASE_WHERE_TYPE_COMPARISON:
    return database_where_is_satisfied_comparison(table, row,
                                                  where.value.comparison);

  case DATABASE_WHERE_TYPE_CONTAINS:
    return database_where_is_satisfied_contains(table, row,
                                                where.value.contains);
  }
}

bool database_where_joined_is_satisfied(struct database_table left_table,
                                        struct database_table right_table,
                                        struct database_row left_row,
                                        struct database_row right_row,
                                        struct database_where_joined where) {
  switch (where.type) {
  case DATABASE_WHERE_TYPE_ALWAYS:
    return true;

  case DATABASE_WHERE_TYPE_LOGIC:
    switch (where.value.logic.operator) {
    case DATABASE_WHERE_LOGIC_OPERATOR_AND:
      return database_where_joined_is_satisfied(left_table, right_table,
                                                left_row, right_row,
                                                *where.value.logic.left) &&
             database_where_joined_is_satisfied(left_table, right_table,
                                                left_row, right_row,
                                                *where.value.logic.right);

    case DATABASE_WHERE_LOGIC_OPERATOR_OR:
      return database_where_joined_is_satisfied(left_table, right_table,
                                                left_row, right_row,
                                                *where.value.logic.left) ||
             database_where_joined_is_satisfied(left_table, right_table,
                                                left_row, right_row,
                                                *where.value.logic.right);
    }

  case DATABASE_WHERE_TYPE_COMPARISON:
    return database_where_joined_is_satisfied_comparison(
        left_table, right_table, left_row, right_row, where.value.comparison);

  case DATABASE_WHERE_TYPE_CONTAINS:
    return database_where_joined_is_satisfied_contains(
        left_table, right_table, left_row, right_row, where.value.contains);

  default:
    return false;
  }
}

void database_where_destroy(struct database_where where) {
  switch (where.type) {
  case DATABASE_WHERE_TYPE_ALWAYS:
  case DATABASE_WHERE_TYPE_COMPARISON:
  case DATABASE_WHERE_TYPE_CONTAINS:
    break;
  case DATABASE_WHERE_TYPE_LOGIC:
    database_where_destroy(*where.value.logic.left);
    free(where.value.logic.left);
    database_where_destroy(*where.value.logic.right);
    free(where.value.logic.right);
    break;
  }
}

void database_where_joined_destroy(struct database_where_joined where) {
  switch (where.type) {
  case DATABASE_WHERE_TYPE_ALWAYS:
  case DATABASE_WHERE_TYPE_COMPARISON:
  case DATABASE_WHERE_TYPE_CONTAINS:
    break;
  case DATABASE_WHERE_TYPE_LOGIC:
    database_where_joined_destroy(*where.value.logic.left);
    free(where.value.logic.left);
    database_where_joined_destroy(*where.value.logic.right);
    free(where.value.logic.right);
    break;
  }
}