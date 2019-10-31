#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <client.h>
#include <utils.h>

#define MAX_CLIENTS 10

int open_listenfd();
int assign_clients_to_sets(fd_set *read_set, fd_set *write_set, struct client *client, int clients_count);
bool accept_new_client(int listenfd, struct client *client);
void clean_clients(struct client *clients, int *clients_count);
void print_clients(struct client *clients, int clients_count, bool verbose);

int main(int argc, char **argv)
{
    int listenfd = open_listenfd();
    if (listenfd < 0)
    {
        fprintf(stderr, ERROR_COLOR "--> Opening listenfd: %s" COLOR_RESET, strerror(errno));
        return -1;
    }

    struct client clients[MAX_CLIENTS];
    int clients_count = 0;

    while (true)
    {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(listenfd, &read_set);

        fd_set write_set;
        FD_ZERO(&write_set);

        int max_fd = assign_clients_to_sets(&read_set, &write_set, clients, clients_count);
        printf("--> Waiting for connection...\n");

        print_clients(clients, clients_count, true);

        select(max(listenfd, max_fd) + 1, &read_set, &write_set, NULL, NULL);

        if (FD_ISSET(listenfd, &read_set) && clients_count < MAX_CLIENTS)
        {
            if (accept_new_client(listenfd, clients + clients_count))
                clients_count++;
            else
                fprintf(stderr, ERROR_COLOR "--> Accepting new client: %s" COLOR_RESET, strerror(errno));
        }

        for (int i = 0; i < clients_count; i++)
        {
            if (FD_ISSET(clients[i].fd, &read_set))
            {
                printf("--> Client (ip: %s, fd: %d) is ready for reading\n", clients[i].ip, clients[i].fd);
                if (clients[i].status == CLIENT_STATUS_READING && !client_read(clients + i))
                    fprintf(stderr, ERROR_COLOR "--> Error reading from client: %d\n" COLOR_RESET, clients[i].fd);
            }

            if (FD_ISSET(clients[i].fd, &write_set))
            {
                printf("--> Client (ip: %s, fd: %d) is ready for writing\n", clients[i].ip, clients[i].fd);
                if (clients[i].status == CLIENT_STATUS_WRITING && !client_write(clients + i))
                    fprintf(stderr, ERROR_COLOR "--> Error writing to client: %d\n" COLOR_RESET, clients[i].fd);
            }
        }

        clean_clients(clients, &clients_count);
    }

    return 0;
}

int open_listenfd()
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    // Create a socket descriptor
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    // Eliminates "Address already in use" error from bind
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
        return -1;

    struct servent *port;
    port = getservbyname("http-alt", "tcp");

    // Listenfd will be an end point for all requests to port
    // on any IP address for this host
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = port->s_port;
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, MAX_CLIENTS) < 0)
        return -1;

    char *ip = inet_ntoa(serveraddr.sin_addr);
    printf("--> Listening on %s:%d\n", ip, ntohs(serveraddr.sin_port));

    return listenfd;
}

int assign_clients_to_sets(fd_set *read_set, fd_set *write_set, struct client *clients, int clients_count)
{
    int max_fd = 0;
    for (int i = 0; i < clients_count; i++)
    {
        if (clients[i].status == CLIENT_STATUS_READING)
        {
            printf("--> Client (%s, %d) is waiting to read\n", clients[i].ip, clients[i].fd);
            FD_SET(clients[i].fd, read_set);
        }

        if (clients[i].status == CLIENT_STATUS_WRITING)
        {
            printf("--> Client (%s, %d) is waiting to write\n", clients[i].ip, clients[i].fd);
            FD_SET(clients[i].fd, write_set);
        }

        max_fd = max(max_fd, clients[i].fd);
    }
    return max_fd;
}

bool accept_new_client(int listenfd, struct client *client)
{
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);

    char *ip = inet_ntoa(client_addr.sin_addr);

    printf("--> Connecting %s\n", ip);

    int fd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
    if (fd < 0)
        return false;

    printf("--> Connection stablished %s\n", ip);

    client_init(client, fd, ip);
    return true;
}

void clean_clients(struct client *clients, int *clients_count)
{
    int k = 0;
    for (int i = 0; i < *clients_count; i++)
    {
        if (clients[i].status == CLIENT_STATUS_DONE)
            client_close(clients + i);
        else
        {
            clients[k] = clients[i];
            k++;
        }
    }
    *clients_count = k;
}

void print_clients(struct client *clients, int clients_count, bool verbose)
{
    printf("--> Currently %d clients\n", clients_count);
    if (verbose)
    {
        for (int i = 0; i < clients_count; i++)
        {
            struct client client = clients[i];
            printf("--> Client {\n");
            printf("  ip: %s\n", client.ip);
            printf("  fd: %d\n", client.fd);
            printf("  status: %d\n", client.status);
            printf("  error: %d\n", client.error);
            printf("  request: %s\n", client.request);
            printf("}\n");
        }
    }
}