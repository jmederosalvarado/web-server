#include <frontend.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <static_files/css1.h>
#include <static_files/css2.h>
#include <static_files/js1.h>
#include <static_files/js2.h>
#include <static_files/js3.h>
#include <static_files/ico.h>
#include <static_files/font1.h>
#include <static_files/font2.h>
#include <static_files/font3.h>
#include <static_files/font4.h>
#include <static_files/font5.h>

void static_sender_init(struct static_sender *static_sender, int fd, char *file)
{
    writer_init(&static_sender->writer, fd, static_sender_send);

    if (strstr(file, "favicon.ico"))
    {
        static_sender->static_content = ico;
        static_sender->len = ico_len;
    }
    else if (strstr(file, "@css1"))
    {
        static_sender->static_content = css1;
        static_sender->len = css1_len;
    }
    else if (strstr(file, "@css2"))
    {
        static_sender->static_content = css2;
        static_sender->len = css2_len;
    }
    else if (strstr(file, "@js1"))
    {
        static_sender->static_content = js1;
        static_sender->len = js1_len;
    }
    else if (strstr(file, "@js2"))
    {
        static_sender->static_content = js2;
        static_sender->len = js2_len;
    }
    else if (strstr(file, "@js3"))
    {
        static_sender->static_content = js3;
        static_sender->len = js3_len;
    }
    else if (strstr(file, "@font1"))
    {
        static_sender->static_content = font1;
        static_sender->len = font1_len;
    }
    else if (strstr(file, "@font2"))
    {
        static_sender->static_content = font2;
        static_sender->len = font2_len;
    }
    else if (strstr(file, "@font3"))
    {
        static_sender->static_content = font3;
        static_sender->len = font3_len;
    }
    else if (strstr(file, "@font4"))
    {
        static_sender->static_content = font4;
        static_sender->len = font4_len;
    }
    else if (strstr(file, "@font5"))
    {
        static_sender->static_content = font5;
        static_sender->len = font5_len;
    }

    static_sender->i = -1;
}

void static_sender_send_header(struct static_sender *static_sender);
int static_sender_send(struct writer *writer)
{
    struct static_sender *sender = (struct static_sender *)writer;

    if (sender->i == -1)
    {
        static_sender_send_header(sender);
        sender->i++;
        return WRITER_STATUS_CONT;
    }

    if (sender->i >= sender->len)
        return WRITER_STATUS_DONE;

    sender->i += write(sender->writer.fd, sender->static_content + sender->i, 4096);
    return WRITER_STATUS_CONT;
}

bool is_static(char *path)
{
    if (strstr(path, "favicon.ico") || strstr(path, "@css1") || strstr(path, "@css2") ||
        strstr(path, "@js1") || strstr(path, "@js2") || strstr(path, "@js3") ||
        strstr(path, "@font1") || strstr(path, "@font2") || strstr(path, "@font3") ||
        strstr(path, "@font4") || strstr(path, "@font5"))
        return true;
    return false;
}

void static_sender_send_header(struct static_sender *static_sender)
{
    char *header =
        "HTTP/1.0 200 OK\r\n"
        "Server: Web Server\r\n\r\n";

    write(static_sender->writer.fd, header, strlen(header) * sizeof(char));
}