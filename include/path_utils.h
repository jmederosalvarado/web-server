#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <path.h>

char *root;
void set_root(char *r);
char *get_root();

int is_dir(const char *path);
int is_file(const char *path);

void get_info(struct path *p);

#endif