#include "logger.h"
#include "models.h"
#include "models_serialization.h"
#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  debug("Client start");

  char read_buffer[2048];

  while (1) {
    size_t curr_length = 0;
    int getchar_ret;
    while ((getchar_ret = getchar()) != EOF) {
      char curr_char = (char)getchar_ret;
      read_buffer[curr_length++] = curr_char;
      if (curr_char == ';')
        break;
    }

    // Add null-terminator
    read_buffer[curr_length] = '\0';

    if (strcmp(read_buffer, "exit;") == 0)
      break;

    struct parsing_result parsed = parse(read_buffer);
    if (parsed.status == PARSING_STATUS_ERROR)
      break;

    char *json_string = serialize(parsed.value);
    if (json_string == NULL)
      break;

    printf("%s\n", json_string);
  }

  return EXIT_SUCCESS;
}