#ifndef ITMO_LOW_LEVEL_PROGRAMMING_LAB1_LOGGER_H
#define ITMO_LOW_LEVEL_PROGRAMMING_LAB1_LOGGER_H

#define debug(...)                                                             \
  logger_log(LOGGER_DEBUG, TARGETNAME, FILENAME, __LINE__, __VA_ARGS__)

#define warn(...)                                                              \
  logger_log(LOGGER_WARN, TARGETNAME, FILENAME, __LINE__, __VA_ARGS__)

enum logger_level { LOGGER_DEBUG = 0, LOGGER_WARN };

void logger_log(enum logger_level level, const char *target, const char *file,
                int line, const char *format, ...);

#endif // ITMO_LOW_LEVEL_PROGRAMMING_LAB1_LOGGER_H