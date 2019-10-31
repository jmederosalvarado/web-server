#ifndef WRITER_H
#define WRITER_H

#define WRITER_STATUS_DONE 1
#define WRITER_STATUS_CONT 2

struct writer
{
    int fd;
    int (*write)();
};

void writer_init(struct writer *writer, int fd, int (*write)(struct writer *writer));

#endif