#include <stdlib.h>
#include <client.h>

void client_init(struct client *client, int fd, int status)
{
    client->fd = fd;
    client->status = status;
    client->request = NULL;
}

void client_close(struct client *client)
{
    client->fd = -1;
    client->status = CLIENT_STATUS_DONE;
    free(client->request);
}

void client_read(struct client *client)
{
}

void client_write(struct client *client)
{
}