#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/select.h>

#include <client.h>

#define MAX_CLIENTS 10

int open_listenfd(int port);
int assign_clients_to_sets(fd_set *read_set, fd_set *write_set, struct client *clients);

int main(int argc, char **argv)
{
    int listenfd = open_listenfd(8080);
    if (listenfd < 0)
    {
        perror("Opening listenfd");
        return -1;
    }
    printf("--> Listening on <ip>:<port>");

    struct client clients[MAX_CLIENTS];

    while (true)
    {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(listenfd, &read_set);

        fd_set write_set;
        FD_ZERO(&write_set);

        int max_fd = assign_clients_to_sets(&read_set, &write_set, clients);
    }

    return 0;
}

int open_listenfd(int port)
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    // Create a socket descriptor
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    // Eliminates "Address already in use" error from bind
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
        return -1;

    // Listenfd will be an end point for all requests to port
    // on any IP address for this host
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, MAX_CLIENTS) < 0)
        return -1;

    return listenfd;
}

int assign_clients_to_sets(fd_set *read_set, fd_set *write_set, struct client *clients)
{
    int max_fd = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].fd < 0)
            continue;
        if (clients[i].status == CLIENT_STATUS_READING)
            FD_SET(clients[i].fd, read_set);
        else if (clients[i].status == CLIENT_STATUS_WRITING)
            FD_SET(clients[i].fd, write_set);

        max_fd = max(max_fd, clients[i].fd);
    }
    return max_fd;
}