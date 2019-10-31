#ifndef PATH_H
#define PATH_H

struct path
{
    char name[1024];
    char type[1024];
    char permissions[1024];
    char moddate[1024];
    int size;
};

void path_init(struct path *path, char *name);

#endif