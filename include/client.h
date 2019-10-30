#ifndef CLIENT_H
#define CLIENT_H

#define CLIENT_STATUS_READING 1
#define CLIENT_STATUS_WRITING 2
#define CLIENT_STATUS_DONE 3

#define CLIENT_ERROR_NONE 0
#define CLIENT_ERROR_READ 1
#define CLIENT_ERROR_METHOD 2

#include <stdbool.h>

struct client
{
    int fd;
    int status;
    int error;
    char request[1024];
    char ip[20];
};

void client_init(struct client *client, int fd, char *ip);
void client_close(struct client *client);
bool client_read(struct client *client);
bool client_write(struct client *client);

#endif