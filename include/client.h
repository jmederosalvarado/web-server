#ifndef CLIENT_H
#define CLIENT_H

#define CLIENT_STATUS_DONE 0
#define CLIENT_STATUS_READING 1
#define CLIENT_STATUS_WRITING 2

struct client
{
    int fd;
    int status;
    char *request;
};

void client_init(struct client *client, int fd, int status);
void client_close(struct client *client);
void client_read(struct client *client);
void client_write(struct client *client);

#endif