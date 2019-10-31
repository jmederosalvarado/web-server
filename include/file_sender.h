#ifndef FILE_SENDER_H
#define FILE_SENDER_H

#include <writer.h>

struct file_sender
{
    struct writer writer;
    char request[1024];
    int read_fd;
};

void file_sender_init(struct file_sender *file_sender, int fd, char *request);
int file_sender_send(struct writer *writer);

#endif