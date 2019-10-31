#ifndef SORT_UTILS_H
#define SORT_UTILS_H

#include <path.h>

void quick_sort(struct path *v, int left, int right, int (*comp)(struct path *, struct path *));

int sort_by_name(struct path *path1, struct path *path2);
int sort_by_type(struct path *path1, struct path *path2);
int sort_by_size(struct path *path1, struct path *path2);
int sort_by_permissions(struct path *path1, struct path *path2);
int sort_by_moddate(struct path *path1, struct path *path2);

#endif