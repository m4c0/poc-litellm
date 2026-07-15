#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

__attribute__((format(printf, 1, 2)))
static inline int log_error(const char * msg, ...) {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
  return 1;
}

__attribute__((format(printf, 1, 2)))
static inline int log_info(const char * msg, ...) {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
  return 0;
}

#endif

