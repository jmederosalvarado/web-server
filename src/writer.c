#include <writer.h>

void writer_init(struct writer *writer, int fd, int (*write)(struct writer *writer))
{
    writer->fd = fd;
    writer->write = write;
}