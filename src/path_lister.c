#include <path_lister.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <sort_utils.h>
#include <path_utils.h>

#define PATH_LISTER_STATUS_LIST 1
#define PATH_LISTER_STATUS_SORT 2
#define PATH_LISTER_STATUS_SEND 3

int scandir(const char *dirp, struct dirent ***namelist,
            int (*filter)(const struct dirent *dir),
            int (*compar)(const struct dirent **dira, const struct dirent **dirb));

int alphasort(const struct dirent **a, const struct dirent **b);

void path_lister_init(struct path_lister *path_lister, int fd, char *request)
{
    path_lister->paths = NULL;
    path_lister->paths_count = 0;
    path_lister->index = -1;
    path_lister->dir = NULL;
    path_lister->request = request;
    path_lister->status = PATH_LISTER_STATUS_LIST;
    writer_init(&path_lister->writer, fd, path_lister_write);
}

int path_lister_list(struct path_lister *path_lister);
int path_lister_sort(struct path_lister *path_lister);
int path_lister_send(struct path_lister *path_lister);
int path_lister_write(struct writer *writer)
{
    struct path_lister *path_lister = (struct path_lister *)writer;

    if (path_lister->status == PATH_LISTER_STATUS_LIST)
        return path_lister_list(path_lister);

    if (path_lister->status == PATH_LISTER_STATUS_SORT)
        return path_lister_sort(path_lister);

    if (path_lister->status == PATH_LISTER_STATUS_SEND)
        return path_lister_send(path_lister);
}

int path_lister_list(struct path_lister *path_lister)
{
    struct dirent **dirents;

    int paths_count = scandir(path_lister->request, &dirents, NULL, alphasort);

    if (path_lister->paths != NULL)
        free(path_lister->paths);

    path_lister->paths = malloc(paths_count * sizeof(struct path));

    for (int i = 0; i < paths_count; i++)
    {
        path_init(path_lister->paths + i, dirents[i]->d_name);
        get_info(path_lister->paths + i);
        free(dirents[i]);
    }
    path_lister->paths_count = paths_count;
    free(dirents);

    path_lister->status = PATH_LISTER_STATUS_SORT;
    return WRITER_STATUS_CONT;
}

int path_lister_sort(struct path_lister *path_lister)
{
    quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_permissions);
    path_lister->status = PATH_LISTER_STATUS_SEND;
    return WRITER_STATUS_CONT;
}

void send_header(struct path_lister *path_lister);
int path_lister_send(struct path_lister *path_lister)
{
    if (path_lister->index == -1)
    {
        send_header(path_lister);
        path_lister->index++;
        return WRITER_STATUS_CONT;
    }

    if (path_lister->index == path_lister->paths_count)
    {
        return WRITER_STATUS_DONE;
    }

    struct path path = path_lister->paths[path_lister->index];

    char path_link[1024];
    sprintf(path_link, "%s%s", path.name, is_dir(path.name) ? "/" : "");

    char response[2048];
    sprintf(response, "<li><a href=\"%s\">%s %s %d %s %s</a></li>", path_link,
            path.name,
            path.type,
            path.size,
            path.moddate,
            path.permissions);

    write(path_lister->writer.fd, response, strlen(response) * sizeof(char));
    path_lister->index++;

    return WRITER_STATUS_CONT;
}

void send_header(struct path_lister *path_lister)
{
    char *header =
        "HTTP/1.0 200 OK\r\n"
        "Server: Web Server\r\n"
        "Content-Type: text/html\r\n\r\n";

    write(path_lister->writer.fd, header, strlen(header) * sizeof(char));
}
