#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/select.h>

#define MAX_CLIENTS 10

int open_listenfd(int port);
int open_clientfd(char *hostname, int port);

int main(int argc, char **argv)
{
    int listenfd = open_listenfd(8080);
    if (listenfd < 0)
    {
        perror("Opening listenfd");
        return -1;
    }
    printf("Listening on <ip>:<port>");

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

int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; // Check errno for cause of error

    // Fill in the server’s IP address and port
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; // Check h_errno for cause of error

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    // Establish a connection with the server
    if (connect(clientfd, (struct serveraddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    return clientfd;
}