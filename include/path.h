#ifndef PATH_H
#define PATH_H

struct path
{
    char name[1024];
};

void path_init(struct path *path, char *name);

#endif