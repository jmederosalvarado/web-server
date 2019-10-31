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

void path_lister_init(struct path_lister *path_lister, int fd, struct request request)
{
    path_lister->paths = NULL;
    path_lister->paths_count = 0;
    path_lister->paths_capacity = 10;
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
    if (path_lister->dir == NULL)
    {
        path_lister->dir = opendir(path_lister->request.path);
        path_lister->paths = malloc(path_lister->paths_capacity * sizeof(struct path));
    }

    struct dirent *dirent = readdir(path_lister->dir);

    if (dirent == NULL)
    {
        closedir(path_lister->dir);
        path_lister->status = PATH_LISTER_STATUS_SORT;
        return WRITER_STATUS_CONT;
    }

    if (path_lister->paths_capacity == path_lister->paths_count)
    {
        path_lister->paths_capacity *= 2;
        path_lister->paths = realloc(path_lister->paths, sizeof(struct path) * path_lister->paths_capacity);
    }

    struct path *path = path_lister->paths + path_lister->paths_count;
    path_init(path, dirent->d_name);
    get_info(path);

    path_lister->paths_count++;

    return WRITER_STATUS_CONT;
}

int path_lister_sort(struct path_lister *path_lister)
{
    if (!strcmp(path_lister->request.orderby, "name"))
        quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_name);
    if (!strcmp(path_lister->request.orderby, "permissions"))
        quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_permissions);
    if (!strcmp(path_lister->request.orderby, "type"))
        quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_type);
    if (!strcmp(path_lister->request.orderby, "moddate"))
        quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_moddate);
    if (!strcmp(path_lister->request.orderby, "size"))
        quick_sort(path_lister->paths, 0, path_lister->paths_count - 1, sort_by_size);

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
        free(path_lister->paths);
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
