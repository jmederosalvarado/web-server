#include <sort_utils.h>
#include <string.h>
#include <stddef.h>

int sort_by_name(struct path *path1, struct path *path2)
{
    return strcmp(path1->name, path2->name);
}

int sort_by_type(struct path *path1, struct path *path2)
{
    return strcmp(path1->type, path2->type);
}

int sort_by_size(struct path *path1, struct path *path2)
{
    if (path1->size > path2->size)
        return 1;

    if (path1->size < path2->size)
        return -1;

    return 0;
}

int sort_by_permissions(struct path *path1, struct path *path2)
{
    return strcmp(path1->permissions, path2->permissions);
}

int sort_by_moddate(struct path *path1, struct path *path2)
{
    return strcmp(path1->moddate, path2->moddate);
}

void swap(struct path *v, int, int);

void quick_sort(struct path *v, int left, int right, int (*comp)(struct path *, struct path *))
{
    int i, last;

    if (left >= right)
        return;
    swap(v, left, (left + right) / 2);
    last = left;
    for (i = left + 1; i <= right; i++)
        if ((*comp)(v + i, v + left) < 0)
            swap(v, ++last, i);
    swap(v, left, last);
    quick_sort(v, left, last - 1, comp);
    quick_sort(v, last + 1, right, comp);
}

void swap(struct path *v, int i, int j)
{
    struct path temp;

    memcpy(&temp, v + i, sizeof(struct path));
    memcpy(v + i, v + j, sizeof(struct path));
    memcpy(v + j, &temp, sizeof(struct path));
}
