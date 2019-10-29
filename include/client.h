#ifndef CLIENT_H
#define CLIENT_H

#define CLIENT_STATUS_READING 1
#define CLIENT_STATUS_WRITING 2

struct client
{
    int fd;
    int status;
};

void client_init(int fd, int status);

#endif