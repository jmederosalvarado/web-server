#include <stdio.h>
#include <stdlib.h>
#include <client.h>
#include <utils.h>
#include <unistd.h>
#include <string.h>

void client_init(struct client *client, int fd, char *ip)
{
    client->fd = fd;
    client->error = CLIENT_ERROR_NONE;
    client->status = CLIENT_STATUS_READING;
    client->request[0] = '\0';
    strcpy(client->ip, ip);
}

void client_close(struct client *client)
{
    printf("--> Closing client %s\n", client->ip);

    close(client->fd);
    client->fd = -1;
    client->error = CLIENT_ERROR_NONE;
    client->status = CLIENT_STATUS_DONE;
    client->request[0] = '\0';
}

bool client_read(struct client *client)
{
    printf("--> Reading from client %s\n", client->ip);

    char buf[4096] = "\0";
    int read_count = read_line(client->fd, buf, 4096);

    int matched = sscanf(buf, "GET %s %s", client->request, NULL);

    if (!matched)
    {
        fprintf(stderr, ERROR_COLOR "--> Method not allowed %s\n" COLOR_RESET, buf);
        client->error = CLIENT_ERROR_METHOD;
    }

    if (read_count == 0)
        client->status = CLIENT_STATUS_DONE;
    else
    {
        client->status = CLIENT_STATUS_WRITING;
        if (read_count < 0)
        {
            client->error = CLIENT_ERROR_READ;
            return false;
        }
    }
    return true;
}

bool client_write(struct client *client)
{
    printf("--> Writing to client %s\n", client->ip);

    char *body =
        "<html>\n"
        "<title> Explorer </title>\n"
        "<body>\n"
        "<h1>Hello from server</h1>\n"
        "</body>\n"
        "</html>\n";

    char header[4096] = "\0";

    strcat(header, "HTTP/1.0 200 OK\n");
    strcat(header, "Server: Web Server\n");

    char content_length[20] = "\0";
    sprintf(content_length, "Content-length: %d\n", strlen(body) * sizeof(char));
    strcat(header, content_length);

    strcat(header, "Content-type: text/html\n\n");

    if (write(client->fd, header, strlen(header) * sizeof(char)) < 0)
        return false;
    if (write(client->fd, body, strlen(body) * sizeof(char)) < 0)
        return false;

    client->status = CLIENT_STATUS_READING;
    return true;
}