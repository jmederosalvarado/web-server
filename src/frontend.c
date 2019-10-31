#include <frontend.h>
#include <string.h>
#include <unistd.h>

#include <static_files/css1.h>
#include <static_files/css2.h>
#include <static_files/js1.h>
#include <static_files/js2.h>
#include <static_files/js3.h>
#include <static_files/ico.h>

void static_sender_init(struct static_sender *static_sender, int fd, char *file)
{
    writer_init(&static_sender->writer, fd, static_sender_send);

    if (!strstr(file, "favico.ico"))
    {
        static_sender->static_content = ico;
        static_sender->len = ico_len;
    }
    if (!strcmp(file, "@css1"))
    {
        static_sender->static_content = css1;
        static_sender->len = css1_len;
    }
    if (!strcmp(file, "@css2"))
    {
        static_sender->static_content = css2;
        static_sender->len = css2_len;
    }
    if (!strcmp(file, "@js1"))
    {
        static_sender->static_content = js1;
        static_sender->len = js1_len;
    }
    if (!strcmp(file, "@js2"))
    {
        static_sender->static_content = js2;
        static_sender->len = js2_len;
    }
    if (!strcmp(file, "@js3"))
    {
        static_sender->static_content = js3;
        static_sender->len = js3_len;
    }

    static_sender->i = 0;
}

int static_sender_send(struct writer *writer)
{
    struct static_sender *sender = (struct static_sender *)writer;

    if (sender->i == sender->len)
        return WRITER_STATUS_DONE;

    write(sender->writer.fd, sender->static_content, 4096);
    return WRITER_STATUS_DONE;
}

bool is_static(char *path)
{
    if (!strcmp(path, "/favico.ico") || !strcmp(path, "@css1") || !strcmp(path, "@css2") ||
        !strcmp(path, "@js1") || !strcmp(path, "@js2") || !strcmp(path, "@js3"))
        return true;
    return false;
}