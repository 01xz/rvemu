#ifndef RVEMU_UTILS_H_
#define RVEMU_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FATALF(fmt, ...)                                                       \
  (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), \
   exit(1))
#define FATAL(msg) FATALF("%s", msg)

#endif  // RVEMU_UTILS_H_
