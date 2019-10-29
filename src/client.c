#include <stdio.h>
#include <stdlib.h>
#include <client.h>
#include <utils.h>

void client_init(struct client *client, int fd)
{
    client->fd = fd;
    client->error = CLIENT_ERROR_NONE;
    client->status = CLIENT_STATUS_READING;
    client->request[0] = '\0';
}

void client_close(struct client *client)
{
    client->fd = -1;
    client->status = CLIENT_STATUS_DONE;
    client->request[0] = '\0';
}

void client_read(struct client *client)
{
    int connectfd = client->fd;

    char buf[4096];
    int read_count = read_line(client->fd, buf, 4096);

    int matched = sscanf(buf, "GET %s %s", client->request, NULL);

    if (!matched)
    {
        fprintf(stderr, "--> Method not allowed");
        client->error = CLIENT_ERROR_METHOD;
    }

    if (read_count == 0)
        client->status = CLIENT_STATUS_DONE;
    else
    {
        client->status = CLIENT_STATUS_WRITING;
        if (read_count < 0)
            client->error = CLIENT_ERROR_READ;
    }
}

void client_write(struct client *client)
{
}