#include "database_table.h"
#include <assert.h>
#include <stdlib.h>

void database_table_destroy(struct database_table table) {
  if (table.data) {
    free(table.data);
  }

  database_attributes_destroy(table.attributes);
}
