#ifndef LOW_LEVEL_PROGRAMMING_LAB3_HANDLERS_H
#define LOW_LEVEL_PROGRAMMING_LAB3_HANDLERS_H

#include "database.h"
#include "models.h"

char *handle_create_request(struct database *database,
                            struct sql_create_statement statement);

char *handle_drop_request(struct database *database,
                          struct sql_drop_statement statement);

char *handle_insert_request(struct database *database,
                            struct sql_insert_statement statement);

char *handle_select_request(struct database *database,
                            struct sql_select_statement statement);

char *handle_delete_request(struct database *database,
                            struct sql_delete_statement statement);

char *handle_update_request(struct database *database,
                            struct sql_update_statement statement);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_HANDLERS_H
