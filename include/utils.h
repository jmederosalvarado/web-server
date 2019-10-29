#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void array_shift_left(void *array, void *array_end, size_t size);
int read_line(int fd, char *buf, size_t n);

#endif