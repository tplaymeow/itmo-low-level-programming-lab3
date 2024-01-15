#include "database_row.h"
#include <stdlib.h>

void database_row_destroy(struct database_row row) {
  if (row.data) {
    free(row.data);
  }

  database_attribute_values_destroy(row.values);
}