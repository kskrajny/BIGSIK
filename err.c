#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "err.h"

void syserr(const char *fmt, ...) {
  va_list fmt_args;
  int err;

  fprintf(stderr, "ERROR: ");
  err = errno;

  va_start(fmt_args, fmt);
  if (vfprintf(stderr, fmt, fmt_args) < 0) {
    fprintf(stderr, " (also error in syserr) ");
  }
  va_end(fmt_args);
  fprintf(stderr, " (%d; %s)\n", err, strerror(err));
  exit(EXIT_FAILURE);
}
