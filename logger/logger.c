#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

static void print_log(const char *level, const char *target, const char *file,
                      int line, const char *format, va_list arguments) {
  printf("[%s]\t%s:%s:%d: ", level, target, file, line);
  vprintf(format, arguments);
  printf("\n");
}

void logger_log(enum logger_level level, const char *target, const char *file,
                int line, const char *format, ...) {
  switch (level) {
  case LOGGER_DEBUG: {
#ifndef NDEBUG
    va_list arguments;
    va_start(arguments, format);
    print_log("DEBUG", target, file, line, format, arguments);
    va_end(arguments);
#endif
  } break;
  case LOGGER_WARN: {
    va_list arguments;
    va_start(arguments, format);
    print_log("WARNING", target, file, line, format, arguments);
    va_end(arguments);
  } break;
  }
}