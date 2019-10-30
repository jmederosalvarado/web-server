#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define ERROR_COLOR "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

void array_shift_left(void *array, void *array_end, size_t size);
int read_line(int fd, char *buf, size_t n);
int max(int a, int b);

#endif