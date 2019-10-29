#include <client.h>

void client_init(struct client *client, int fd, int status)
{
    client->fd = fd;
    client->status = status;
}

void client_read(struct client *client)
{
}

void client_write(struct client *client)
{
}