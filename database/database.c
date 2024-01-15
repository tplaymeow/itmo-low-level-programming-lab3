#include "database.h"
#include "logger.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct database {
  struct paging_pager *pager;
};

struct database_file_table_header {
  uint64_t table_name_offset;
  uint64_t attributes_count;
};

struct database_file_table_attribute {
  uint64_t attribute_name_offset;
  uint64_t attribute_type;
};

struct database_file_row_header {
  uint64_t table_name_offset;
};

struct database *database_init(FILE *file) {
  struct database *database = malloc(sizeof(struct database));
  if (database == NULL) {
    return NULL;
  }

  struct paging_pager *pager = paging_pager_init(file);
  if (pager == NULL) {
    return NULL;
  }

  database->pager = pager;
  return database;
}

struct database *database_create_and_init(FILE *file) {
  struct database *database = malloc(sizeof(struct database));
  if (database == NULL) {
    return NULL;
  }

  struct paging_pager *pager = paging_pager_create_and_init(file);
  if (pager == NULL) {
    return NULL;
  }

  database->pager = pager;
  return database;
}

void database_destroy(struct database *database) {
  if (database == NULL) {
    return;
  }
  paging_pager_destroy(database->pager);
  free(database);
}

static uint64_t database_attribute_type_to_uint64[] = {
    [DATABASE_ATTRIBUTE_INTEGER] = 0,
    [DATABASE_ATTRIBUTE_FLOATING_POINT] = 1,
    [DATABASE_ATTRIBUTE_BOOLEAN] = 2,
    [DATABASE_ATTRIBUTE_STRING] = 3,
};

static enum database_attribute_type database_attribute_type_from_uint64[] = {
    [0] = DATABASE_ATTRIBUTE_INTEGER,
    [1] = DATABASE_ATTRIBUTE_FLOATING_POINT,
    [2] = DATABASE_ATTRIBUTE_BOOLEAN,
    [3] = DATABASE_ATTRIBUTE_STRING,
};

struct database_create_table_result
database_create_table(struct database *database,
                      struct database_create_table_request request) {
  if (database == NULL) {
    return (struct database_create_table_result){.success = false};
  }

  const size_t header_size = sizeof(struct database_file_table_header);
  const size_t attribute_size = sizeof(struct database_file_table_attribute);
  const size_t data_size_without_strings =
      header_size + request.attributes.count * attribute_size;

  const size_t table_name_data_size = strlen(request.name) + 1;
  size_t strings_data_size = table_name_data_size;
  for (size_t i = 0; i < request.attributes.count; ++i) {
    strings_data_size +=
        strlen(database_attributes_get(request.attributes, i).name) + 1;
  }

  const size_t data_size = data_size_without_strings + strings_data_size;
  void *data = malloc(data_size);
  if (data == NULL) {
    warn("Data allocation error");
    return (struct database_create_table_result){.success = false};
  }

  size_t data_offset = 0;
  size_t data_strings_offset = data_size_without_strings;

  const struct database_file_table_header header = {
      .attributes_count = request.attributes.count,
      .table_name_offset = data_strings_offset};
  memcpy((char *)data + data_offset, &header, header_size);
  memcpy((char *)data + data_strings_offset, request.name,
         table_name_data_size);
  data_offset += header_size;
  data_strings_offset += table_name_data_size;

  for (size_t i = 0; i < request.attributes.count; ++i) {
    const struct database_attribute attribute =
        database_create_table_request_get_attribute(request, i);
    const struct database_file_table_attribute file_attribute = {
        .attribute_name_offset = data_strings_offset,
        .attribute_type = database_attribute_type_to_uint64[attribute.type]};
    const size_t attribute_name_size = strlen(attribute.name) + 1;
    memcpy((char *)data + data_offset, &file_attribute, attribute_size);
    memcpy((char *)data + data_strings_offset, attribute.name,
           attribute_name_size);
    data_offset += attribute_size;
    data_strings_offset += attribute_name_size;
  }

  assert(data_offset == data_size_without_strings);
  assert(data_strings_offset == data_size);

  struct paging_write_result write_result =
      paging_write(database->pager, PAGING_TYPE_1, data, data_size);
  if (!write_result.success) {
    warn("Write data to pager error");
    free(data);
    return (struct database_create_table_result){.success = false};
  }

  free(data);

  return (struct database_create_table_result){.success = true};
}

static struct database_table
database_table_from_file_data(struct paging_info page_info, void *data) {
  const struct database_file_table_header *header = data;
  char *table_name = (char *)data + header->table_name_offset;

  const struct database_file_table_attribute *file_attributes =
      (struct database_file_table_attribute
           *)((char *)data + sizeof(struct database_file_table_header));

  struct database_attributes attributes =
      database_attributes_create(header->attributes_count);
  for (size_t i = 0; i < header->attributes_count; ++i) {
    const struct database_attribute attribute = {
        .name = (char *)data + file_attributes[i].attribute_name_offset,
        .type = database_attribute_type_from_uint64[file_attributes[i]
                                                        .attribute_type]};
    database_attributes_set(attributes, i, attribute);
  }

  return (struct database_table){.data = data,
                                 .name = table_name,
                                 .page_info = page_info,
                                 .attributes = attributes};
}

struct database_get_table_result
database_get_table_with_name(const struct database *database,
                             const char *name) {
  if (database == NULL) {
    return (struct database_get_table_result){.success = false};
  }

  void *data = NULL;
  struct paging_read_result read_result =
      paging_read_first(database->pager, PAGING_TYPE_1, &data);

  while (read_result.success) {
    const struct database_table table =
        database_table_from_file_data(read_result.info, data);
    if (strcmp(table.name, name) == 0) {
      return (struct database_get_table_result){.success = true,
                                                .table = table};
    }

    free(data);
    read_result = paging_read_next(database->pager, read_result.info, &data);
  }

  return (struct database_get_table_result){.success = false};
}

struct database_drop_table_result
database_drop_table(struct database *database, struct database_table table) {
  struct database_select_row_result select_result =
      database_select_row_first(database, table, DATABASE_WHERE_ALWAYS);
  while (select_result.success) {
    const struct database_remove_row_result remove_row_result =
        database_remove_row(database, select_result.row);
    if (!remove_row_result.success) {
      warn("Row removing error");
      database_row_destroy(select_result.row);
      return (struct database_drop_table_result){.success = false};
    }

    select_result =
        database_select_row_first(database, table, DATABASE_WHERE_ALWAYS);
  }

  const struct paging_remove_result remove_table_result =
      paging_remove(database->pager, table.page_info);
  if (!remove_table_result.success) {
    warn("Table removing error");
    return (struct database_drop_table_result){.success = false};
  }

  database_table_destroy(table);
  return (struct database_drop_table_result){.success = true};
}

struct database_insert_row_result
database_insert_row(struct database *database, struct database_table table,
                    struct database_insert_row_request request) {
  const size_t header_data_size = sizeof(struct database_file_row_header);
  const size_t integer_data_size = sizeof(int64_t);
  const size_t floating_point_data_size = sizeof(double);
  const size_t boolean_data_size = sizeof(uint64_t);
  const size_t table_name_data_size = strlen(table.name) + 1;

  size_t data_size_without_strings = header_data_size;
  size_t strings_data_size = table_name_data_size;
  for (size_t i = 0; i < table.attributes.count; i++) {
    switch (database_attributes_get(table.attributes, i).type) {
    case DATABASE_ATTRIBUTE_INTEGER:
      data_size_without_strings += integer_data_size;
      break;
    case DATABASE_ATTRIBUTE_FLOATING_POINT:
      data_size_without_strings += floating_point_data_size;
      break;
    case DATABASE_ATTRIBUTE_BOOLEAN:
      data_size_without_strings += boolean_data_size;
      break;
    case DATABASE_ATTRIBUTE_STRING:
      data_size_without_strings += sizeof(uint64_t);
      strings_data_size +=
          strlen(database_insert_row_request_get_value(request, i).string) + 1;
      break;
    default:
      break;
    }
  }

  const size_t data_size = data_size_without_strings + strings_data_size;
  void *data = malloc(data_size);
  if (data == NULL) {
    warn("Alloc data error");
    return (struct database_insert_row_result){.success = false};
  }

  size_t data_offset = 0;
  size_t data_strings_offset = data_size_without_strings;

  const struct database_file_row_header header = {.table_name_offset =
                                                      data_strings_offset};
  memcpy((char *)data + data_offset, &header, header_data_size);
  memcpy((char *)data + data_strings_offset, table.name, table_name_data_size);
  data_offset += header_data_size;
  data_strings_offset += table_name_data_size;

  for (size_t i = 0; i < table.attributes.count; i++) {
    switch (database_attributes_get(table.attributes, i).type) {
    case DATABASE_ATTRIBUTE_INTEGER: {
      const int64_t value =
          database_insert_row_request_get_value(request, i).integer;
      memcpy((char *)data + data_offset, &value, integer_data_size);
      data_offset += integer_data_size;
    } break;
    case DATABASE_ATTRIBUTE_FLOATING_POINT: {
      const double value =
          database_insert_row_request_get_value(request, i).floating_point;
      memcpy((char *)data + data_offset, &value, floating_point_data_size);
      data_offset += floating_point_data_size;
    } break;
    case DATABASE_ATTRIBUTE_BOOLEAN: {
      const uint64_t value =
          database_insert_row_request_get_value(request, i).boolean;
      memcpy((char *)data + data_offset, &value, boolean_data_size);
      data_offset += boolean_data_size;
    } break;
    case DATABASE_ATTRIBUTE_STRING: {
      const char *value =
          database_insert_row_request_get_value(request, i).string;
      const size_t string_data_size = strlen(value) + 1;
      memcpy((char *)data + data_offset, &data_strings_offset,
             sizeof(uint64_t));
      memcpy((char *)data + data_strings_offset, value, string_data_size);
      data_offset += sizeof(uint64_t);
      data_strings_offset += string_data_size;
    } break;
    default:
      break;
    }
  }

  assert(data_offset == data_size_without_strings);
  assert(data_strings_offset == data_size);

  struct paging_write_result write_result =
      paging_write(database->pager, PAGING_TYPE_2, data, data_size);
  if (!write_result.success) {
    warn("Write data to pager error");
    free(data);
    return (struct database_insert_row_result){.success = false};
  }

  free(data);

  return (struct database_insert_row_result){.success = true};
}

struct database_select_row_result
database_row_values_from_file_data(struct database_table table,
                                   struct database_where where,
                                   struct paging_info paging_info, void *data) {
  const size_t header_data_size = sizeof(struct database_file_row_header);
  const size_t integer_data_size = sizeof(int64_t);
  const size_t floating_point_data_size = sizeof(double);
  const size_t boolean_data_size = sizeof(uint64_t);

  size_t data_offset = 0;

  const struct database_file_row_header *header = data;
  data_offset += header_data_size;

  const char *table_name = (char *)data + header->table_name_offset;
  if (strcmp(table_name, table.name) != 0) {
    return (struct database_select_row_result){.success = false};
  }

  struct database_attribute_values values =
      database_attribute_values_create(table.attributes.count);

  for (size_t i = 0; i < table.attributes.count; i++) {
    switch (database_attributes_get(table.attributes, i).type) {
    case DATABASE_ATTRIBUTE_INTEGER: {
      const union database_attribute_value value = {
          .integer = *(int64_t *)((char *)data + data_offset)};
      database_attribute_values_set(values, i, value);
      data_offset += integer_data_size;
    } break;
    case DATABASE_ATTRIBUTE_FLOATING_POINT: {
      const union database_attribute_value value = {
          .floating_point = *(double *)((char *)data + data_offset)};
      database_attribute_values_set(values, i, value);
      data_offset += floating_point_data_size;
    } break;
    case DATABASE_ATTRIBUTE_BOOLEAN: {
      const union database_attribute_value value = {
          .boolean = *(uint64_t *)((char *)data + data_offset)};
      database_attribute_values_set(values, i, value);
      data_offset += boolean_data_size;
    } break;
    case DATABASE_ATTRIBUTE_STRING: {
      const uint64_t string_offset =
          *((uint64_t *)((char *)data + data_offset));
      const union database_attribute_value value = {.string = (char *)data +
                                                              string_offset};
      database_attribute_values_set(values, i, value);
      data_offset += sizeof(uint64_t);
    } break;
    default:
      break;
    }
  }

  const struct database_row row = {
      .data = data, .paging_info = paging_info, .values = values};
  if (!database_where_is_satisfied(table, row, where)) {
    database_attribute_values_destroy(values);
    return (struct database_select_row_result){.success = false};
  }

  return (struct database_select_row_result){.success = true, .row = row};
}

struct database_select_row_result
database_select_row_first(const struct database *database,
                          struct database_table table,
                          struct database_where where) {
  if (database == NULL) {
    return (struct database_select_row_result){.success = false};
  }

  void *data = NULL;
  struct paging_read_result read_result =
      paging_read_first(database->pager, PAGING_TYPE_2, &data);

  while (read_result.success) {
    const struct database_select_row_result select_result =
        database_row_values_from_file_data(table, where, read_result.info,
                                           data);
    if (select_result.success) {
      return select_result;
    }

    free(data);
    read_result = paging_read_next(database->pager, read_result.info, &data);
  }

  return (struct database_select_row_result){.success = false};
}

struct database_select_row_result database_select_row_next(
    const struct database *database, struct database_table table,
    struct database_where where, struct database_row previous) {
  if (database == NULL) {
    return (struct database_select_row_result){.success = false};
  }

  void *data = NULL;
  struct paging_read_result read_result =
      paging_read_next(database->pager, previous.paging_info, &data);

  database_row_destroy(previous);

  while (read_result.success) {
    const struct database_select_row_result select_result =
        database_row_values_from_file_data(table, where, read_result.info,
                                           data);
    if (select_result.success) {
      return select_result;
    }

    free(data);
    read_result = paging_read_next(database->pager, read_result.info, &data);
  }

  return (struct database_select_row_result){.success = false};
}

struct database_select_join_result database_select_join_first(
    const struct database *database, struct database_table left_table,
    struct database_table right_table, struct database_join join,
    struct database_where_joined where) {
  if (database == NULL) {
    return (struct database_select_join_result){.success = false};
  }

  struct database_select_row_result left_result =
      database_select_row_first(database, left_table, DATABASE_WHERE_ALWAYS);
  while (left_result.success) {
    struct database_select_row_result right_result =
        database_select_row_first(database, right_table, DATABASE_WHERE_ALWAYS);
    while (right_result.success) {
      if (database_join_is_satisfied(left_table, left_result.row, right_table,
                                     right_result.row, join) &&
          database_where_joined_is_satisfied(left_table, right_table,
                                             left_result.row, right_result.row,
                                             where)) {
        return (struct database_select_join_result){
            .success = true,
            .left_row = left_result.row,
            .right_row = right_result.row,
        };
      }

      right_result = database_select_row_next(
          database, right_table, DATABASE_WHERE_ALWAYS, right_result.row);
    }

    left_result = database_select_row_next(
        database, left_table, DATABASE_WHERE_ALWAYS, left_result.row);
  }

  return (struct database_select_join_result){.success = false};
}

struct database_select_join_result database_select_join_next(
    const struct database *database, struct database_table left_table,
    struct database_table right_table, struct database_join join,
    struct database_where_joined where, struct database_row previous_left,
    struct database_row previous_right) {
  if (database == NULL) {
    return (struct database_select_join_result){.success = false};
  }

  struct database_select_row_result left_result = {.success = true,
                                                   .row = previous_left};
  struct database_select_row_result right_result = database_select_row_next(
      database, right_table, DATABASE_WHERE_ALWAYS, previous_right);
  while (left_result.success) {
    while (right_result.success) {
      if (database_join_is_satisfied(left_table, left_result.row, right_table,
                                     right_result.row, join) &&
          database_where_joined_is_satisfied(left_table, right_table,
                                             left_result.row, right_result.row,
                                             where)) {
        return (struct database_select_join_result){
            .success = true,
            .left_row = left_result.row,
            .right_row = right_result.row,
        };
      }

      right_result = database_select_row_next(
          database, right_table, DATABASE_WHERE_ALWAYS, right_result.row);
    }

    // TODO: Think about fix. We can select first row when left result failed
    right_result =
        database_select_row_first(database, right_table, DATABASE_WHERE_ALWAYS);

    left_result = database_select_row_next(
        database, left_table, DATABASE_WHERE_ALWAYS, left_result.row);
  }

  // TODO: Think about fix. We can select first row when left result failed
  if (right_result.success) {
    database_row_destroy(right_result.row);
  }

  return (struct database_select_join_result){.success = false};
}

struct database_remove_row_result
database_remove_row(const struct database *database, struct database_row row) {
  if (database == NULL) {
    return (struct database_remove_row_result){.success = false};
  }

  const struct paging_remove_result result =
      paging_remove(database->pager, row.paging_info);
  if (!result.success) {
    warn("Remove row from pager error");
    return (struct database_remove_row_result){.success = false};
  }

  database_row_destroy(row);
  return (struct database_remove_row_result){.success = true};
}