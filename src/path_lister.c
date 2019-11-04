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

void send_initial_body(struct path_lister *path_lister);
void send_final_body(struct path_lister *path_lister);

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

    char path_absolute[1024];
    strcpy(path_absolute, path_lister->request.path);
    strcat(path_absolute, path->name);

    get_info(path_absolute, path);

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
        send_initial_body(path_lister);

        path_lister->index++;
        return WRITER_STATUS_CONT;
    }

    if (path_lister->index == path_lister->paths_count)
    {
        send_final_body(path_lister);
        free(path_lister->paths);
        return WRITER_STATUS_DONE;
    }

    struct path path = path_lister->paths[path_lister->index];

    char path_link[1024];

    char path_absolute[1024];
    strcpy(path_absolute, path_lister->request.path);
    strcat(path_absolute, path.name);

    sprintf(path_link, "%s%s", path.name, is_dir(path_absolute) ? "/" : "");

    if (!strcmp(path.name, ".") || !strcmp(path.name, ".."))
    {
        path_lister->index++;
        return WRITER_STATUS_CONT;
    }

    char response[2048];

    sprintf(response, "<tr>\n");
    /*   */ sprintf(response, "%s<th scope=\"row\">\n", response);
    /*   */ /*   */ sprintf(response, "%s<a href=\"%s\">\n", response, path_link);
    /*   */ /*   */ /*   */ sprintf(response, "%s<span class=\"btn btn-primary\">%s</span>\n", response, path.name);
    /*   */ /*   */ sprintf(response, "%s</a>\n", response);
    /*   */ sprintf(response, "%s</th>\n", response);
    /*   */ sprintf(response, "%s<td>%s</td>\n", response, path.type);
    /*   */ sprintf(response, "%s<td>%s</td>\n", response, path.permissions);
    /*   */ sprintf(response, "%s<td>%d</td>\n", response, path.size);
    /*   */ sprintf(response, "%s<td>%s</td>\n", response, path.moddate);
    /*   */ sprintf(response, "%s</tr>\n", response);

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

void send_initial_body(struct path_lister *path_lister)
{
    char response[4096];
    sprintf(response, "<!doctype html>\n");
    sprintf(response, "%s<html lang=\"en\">\n", response);

    /*   */ sprintf(response, "%s<head>\n", response);

    /*   */ sprintf(response, "%s<meta charset=\"utf -8\">\n", response);
    /*   */ sprintf(response, "%s<meta name=\"viewport\" content =\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n", response);
    /*   */ sprintf(response, "%s<link rel=\"stylesheet\" href=\"@css1.css\">\n", response);
    /*   */ sprintf(response, "%s<link rel=\"stylesheet\" href=\"@css2.css\">\n", response);

    sprintf(response, "%s<title>Explorer</title>\n", response);

    sprintf(response, "%s</head>\n", response);

    sprintf(response, "%s<body>\n", response);

    /*   */ sprintf(response, "%s<div class=\"container mt-5\">\n", response);

    /*   */ /*   */ sprintf(response, "%s<div class=\"card\">\n", response);

    /*   */ /*   */ /*   */ sprintf(response, "%s<div class=\"card-body\">\n", response);

    /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<div class=\"row\">\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<div class=\"col-11  \">\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<h3 class=\"card-title\">Index of %s</h3>\n", response, path_lister->request.path);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<div class=\"col-1\">\n", response);

    char dir[4096];
    strcpy(dir, path_lister->request.path);
    dir[strlen(path_lister->request.path) - 1] = '\0';
    if (strcmp(dir, get_root()))
        /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a href=\"..\" class=\"material-icons\">arrow_back</a>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<table class=\"table\">\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<thead class=\"thead-dark\">\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<tr>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<th scope=\"col\">\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a class=\"btn btn-primary\" href=\"./@orderby-name\">Name</a>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</th>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<th scope=\"col\">\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a class=\"btn btn-primary\" href=\"./@orderby-type\">Type</a>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</th>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<th scope=\"col\">\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a class=\"btn btn-primary\" href=\"./@orderby-permissions\">Permissions</a>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</th>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<th scope=\"col\">\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a class=\"btn btn-primary\" href=\"./@orderby-size\">Size</a>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</th>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<th scope=\"col\">\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<a class=\"btn btn-primary\" href=\"./@orderby-moddate\">Moddate</a>\n", response);
    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</th>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</tr>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</thead>\n", response);

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s<tbody>\n", response);

    write(path_lister->writer.fd, response, strlen(response) * sizeof(char));
}

void send_final_body(struct path_lister *path_lister)
{
    char response[4096];

    /*   */ /*   */ /*   */ /*   */ /*   */ sprintf(response, "</tbody>\n");

    /*   */ /*   */ /*   */ /*   */ sprintf(response, "%s</table>\n", response);

    /*   */ /*   */ /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ sprintf(response, "%s</div>\n", response);

    /*   */ sprintf(response, "%s<script src=\"@js1.js\"></script>\n", response);
    /*   */ sprintf(response, "%s<script src=\"@js2.js\"></script>\n", response);
    /*   */ sprintf(response, "%s<script src=\"@js3.js\"></script>\n", response);
    /*   */ sprintf(response, "%s<script>$(document).ready(function () { $('body').bootstrapMaterialDesign(); });</script>\n", response);

    sprintf(response, "%s</body>\n", response);

    sprintf(response, "%s</html>\n", response);

    write(path_lister->writer.fd, response, strlen(response) * sizeof(char));
}
