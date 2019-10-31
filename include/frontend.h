#ifndef FRONTEND_H
#define FRONTEND_H

#include <stdbool.h>
#include <writer.h>

struct static_sender
{
    struct writer writer;
    char *static_content;
    int len;
    int i;
};

void static_sender_init(struct static_sender *static_sender, int fd, char *file);
int static_sender_send(struct writer *writer);

bool is_static(char *path);

#endif