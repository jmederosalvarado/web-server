#ifndef PATH_LISTER_H
#define PATH_LISTER_H

#include <writer.h>
#include <sys/types.h>
#include <dirent.h>
#include <path.h>
#include <request.h>

struct path_lister
{
    struct writer writer;

    struct path *paths;
    int paths_count;
    int paths_capacity;

    int status;

    DIR *dir;
    int index;
    struct request request;
};

void path_lister_init(struct path_lister *path_lister, int fd, struct request request);
int path_lister_write(struct writer *writer);

#endif