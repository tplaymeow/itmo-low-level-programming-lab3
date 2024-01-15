#include "connection.h"
#include "logger.h"
#include "models.h"
#include "models_serialization.h"
#include "parsing.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  debug("Client start");

  if (argc < 3) {
    printf("Invalid arguments\n");
    return EXIT_FAILURE;
  }

  const char *host = argv[1];
  const int port = atoi(argv[2]);

  while (1) {
    char *input_string = NULL;
    size_t input_length = 0;
    ssize_t read = getdelim(&input_string, &input_length, ';', stdin);
    if (read < 0) {
      warn("Read error");
      return EXIT_FAILURE;
    }

    struct parsing_result parsed = parse(input_string);
    if (parsed.status == PARSING_STATUS_ERROR) {
      printf("Error: Unknown input\n");
      continue;
    }

    if (parsed.value.type == PARSING_TYPE_EXIT) {
      debug("Exit command");
      break;
    }

    char *json_string = serialize(parsed.value.value.statement);
    if (json_string == NULL) {
      warn("Serialization error");
      return EXIT_FAILURE;
    }

    char *response = client_request(host, port, json_string);
    if (response == NULL) {
      warn("Request to server error");
      return EXIT_FAILURE;
    }

    const struct models_deserialization_common_response_result common =
        deserialize_common_response(response);
    const struct models_deserialization_select_response_result select =
        deserialize_select_response(response);
    if (common.type == MODELS_DESERIALIZATION_RESULT_OK) {
      // printf("%s\n", common.value.message);
    } else if (select.type == MODELS_DESERIALIZATION_RESULT_OK) {
      for (size_t i = 0; i < select.value.header.columns_count; i++) {
        printf("%-15s", select.value.header.columns[i]);
      }
      printf("\n");
      for (struct sql_literal_list_list *row = select.value.rows; row != NULL;
           row = row->next) {
        for (struct sql_literal_list *l = row->item; l != NULL; l = l->next) {
          switch (l->item.type) {
          case SQL_DATA_TYPE_INTEGER:
            printf("%-15" PRIi64, l->item.value.integer);
            break;
          case SQL_DATA_TYPE_FLOATING_POINT:
            printf("%-15lf", l->item.value.floating_point);
            break;
          case SQL_DATA_TYPE_BOOLEAN:
            printf("%-15s", l->item.value.boolean ? "true" : "false");
            break;
          case SQL_DATA_TYPE_TEXT:
            printf("%-15s", l->item.value.text);
            break;
          default:
            break;
          }
        }
        printf("\n");
      }
    } else {
      printf("%s\n", response);
    }

    //    struct models_deserialization_result dr = deserialize(json_string);
    //    if (dr.type == MODELS_DESERIALIZATION_RESULT_ERROR) {
    //      debug("Deserialization error");
    //      return EXIT_FAILURE;
    //    }

    //    char *json_string2 = serialize(parsed.value.value.statement);
    //    if (json_string2 == NULL) {
    //      debug("Re-serialization error");
    //      return EXIT_FAILURE;
    //    }

    //    if (strcmp(json_string, json_string2) == 0)
    //      debug("Equal!!!");
    //    else
    //      debug("Not Equal!!!");
  }

  return EXIT_SUCCESS;
}