#ifndef PATH_UTILS_H
#define PATH_UTILS_H

char *root;
void set_root(char *r);
char *get_root();

int is_dir(const char *path);
int is_file(const char *path);

#endif