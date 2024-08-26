// A stream socket client demo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define MAX_SIZE 50

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: ./client.out hostname\n");
        exit(1);
    }

    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv = getaddrinfo(argv[1], PORT, &hints, &servinfo);
    if (rv != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    int sockfd;
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) // go through all structs in list
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    char str[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, p->ai_addr, str, sizeof str);
    printf("Connected to server : %s\n", str);

    freeaddrinfo(servinfo);

    char buffer[MAX_SIZE];
    int numbytes = recv(sockfd, buffer, MAX_SIZE, 0);
    if (numbytes == -1)
    {
        perror("receive");
        exit(1);
    }
    buffer[numbytes] = '\0';
    printf("client received: '%s' (%d)\n", buffer, numbytes);

    close(sockfd);
    printf("client: shutting down...\n");
    return 0;
}