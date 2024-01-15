#include "connection.h"
#include "database.h"
#include "handlers.h"
#include "logger.h"
#include "models_serialization.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

int main(int argc, char **argv) {
  debug("Server start");

  if (argc < 3) {
    printf("Invalid arguments\n");
    return EXIT_FAILURE;
  }

  const int port = atoi(argv[2]);
  struct server *server = server_init(port);
  if (server == NULL) {
    warn("Server init error");
    return EXIT_FAILURE;
  }

  const char *filename = argv[1];
  const bool is_file_exists = access(filename, F_OK) == 0;

  FILE *file = is_file_exists ? fopen(filename, "rb+") : fopen(filename, "ab+");
  if (file == NULL) {
    warn("File open failed. Errno: %d", errno);
    return EXIT_FAILURE;
  }

  struct database *database =
      is_file_exists ? database_init(file) : database_create_and_init(file);
  if (database == NULL) {
    warn("Database init error");
    return EXIT_FAILURE;
  }

  while (1) {
    char *request = server_accept(server);
    if (request == NULL) {
      warn("Failed to accept connection. Skip request");
      continue;
    }

    debug("Accept json: %s", request);

    struct models_deserialization_result deserialization = deserialize(request);
    if (deserialization.type == MODELS_DESERIALIZATION_RESULT_ERROR) {
      warn("Failed to deserialized json. Skip request");
      continue;
    }

    char *response;
    switch (deserialization.value.type) {
    case SQL_STATEMENT_TYPE_CREATE: {
      response =
          handle_create_request(database, deserialization.value.value.create);
    } break;
    case SQL_STATEMENT_TYPE_DROP: {
      response =
          handle_drop_request(database, deserialization.value.value.drop);
    } break;
    case SQL_STATEMENT_TYPE_INSERT: {
      response =
          handle_insert_request(database, deserialization.value.value.insert);
    } break;
    case SQL_STATEMENT_TYPE_SELECT: {
      response =
          handle_select_request(database, deserialization.value.value.select);
    } break;
    case SQL_STATEMENT_TYPE_DELETE: {
      response =
          handle_delete_request(database, deserialization.value.value.delete);
    } break;
    case SQL_STATEMENT_TYPE_UPDATE: {
      response =
          handle_update_request(database, deserialization.value.value.update);
    } break;
    }

    if (response == NULL) {
      warn("Unsupported operation");
      continue;
    }

    const bool send_result = server_send(server, response);
    if (!send_result) {
      warn("Send failed");
      continue;
    }

    debug("Send response: %s", response);
  }

  return EXIT_SUCCESS;
}