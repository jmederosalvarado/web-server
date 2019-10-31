#ifndef PATH_H
#define PATH_H

struct path
{
    char name[1024];
    char type[50];
    char permissions[50];
    char moddate[50];
    int size;
};

void path_init(struct path *path, char *name);

#endif