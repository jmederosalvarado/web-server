#include <string.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <file_sender.h>

void file_sender_init(struct file_sender *file_sender, int fd, char *request)
{
    writer_init(&file_sender->writer, fd, file_sender_send);
    file_sender->read_fd = -1;
    strcpy(file_sender->request, request);
}

void file_sender_send_header(struct file_sender *file_sender);
int file_sender_send(struct writer *writer)
{
    struct file_sender *file_sender = (struct file_sender *)writer;
    if (file_sender->read_fd == -1)
    {
        file_sender->read_fd = open(file_sender->request, O_RDONLY);
        file_sender_send_header(file_sender);
        return WRITER_STATUS_CONT;
    }

    int send_count = sendfile(file_sender->writer.fd, file_sender->read_fd, NULL, 1024);

    if (send_count > 0)
        return WRITER_STATUS_CONT;

    return WRITER_STATUS_DONE;
}

void file_sender_send_header(struct file_sender *file_sender)
{
    char *header =
        "HTTP/1.0 200 OK\r\n"
        "Server: Web Server\r\n"
        "Content-Type: x-download\r\n\r\n";

    write(file_sender->writer.fd, header, strlen(header) * sizeof(char));
}
