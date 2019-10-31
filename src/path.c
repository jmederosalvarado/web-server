#include <path.h>
#include <string.h>

void path_init(struct path *path, char *name)
{
    strcpy(path->name, name);
}