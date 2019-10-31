#include <stdio.h>
#include <stdlib.h>
#include <client.h>
#include <utils.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <path_lister.h>

void client_init(struct client *client, int fd, char *ip)
{
    client->fd = fd;
    client->error = -1;
    client->status = CLIENT_STATUS_READING;
    client->request[0] = '\0';
    client->writer = NULL;
    strcpy(client->ip, ip);
}

void client_close(struct client *client)
{
    printf("--> Closing client %s\n", client->ip);

    close(client->fd);
    client->fd = -1;
    client->error = -1;
    client->status = CLIENT_STATUS_DONE;
    client->request[0] = '\0';
}

bool client_read(struct client *client)
{
    printf("--> Reading from client %s\n", client->ip);

    char buf[4096] = "\0";
    int read_count = read_line(client->fd, buf, 4096);

    printf("--> Read: %d", read_count);

    int matched = sscanf(buf, "GET %s %s", client->request, NULL);

    if (!matched)
    {
        fprintf(stderr, ERROR_COLOR "--> Method not allowed %s\n" COLOR_RESET, buf);
        client->error = 400;
    }

    if (read_count == 0)
        client->status = CLIENT_STATUS_DONE;
    else
    {
        client->status = CLIENT_STATUS_WRITING;
        if (read_count < 0)
        {
            client->error = 500;
            return false;
        }
    }
    return true;
}

int is_dir(const char *path);
int is_file(const char *path);

bool client_write(struct client *client)
{
    printf("--> Writing to client %s\n", client->ip);

    if (client->writer != NULL)
    {
        if ((*client->writer->write)(client->writer) == WRITER_STATUS_DONE)
        {
            client->status = CLIENT_STATUS_DONE;
            free(client->writer);
            client->writer = NULL;
        }
        return true;
    }

    if (is_dir(client->request))
    {
        client->writer = malloc(sizeof(struct path_lister));
        path_lister_init((struct path_lister *)client->writer, client->fd, client->request);
        return true;
    }

    client->status = CLIENT_STATUS_DONE;
    return false;
}

int is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

int is_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}