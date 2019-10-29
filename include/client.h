#ifndef CLIENT_H
#define CLIENT_H

#define CLIENT_STATUS_DONE 0
#define CLIENT_STATUS_READING 1
#define CLIENT_STATUS_WRITING 2

#define CLIENT_ERROR_NONE 0
#define CLIENT_ERROR_READ 1
#define CLIENT_ERROR_METHOD 2

struct client
{
    int fd;
    int status;
    int error;
    char request[1024];
};

void client_init(struct client *client, int fd);
void client_close(struct client *client);
void client_read(struct client *client);
void client_write(struct client *client);

#endif