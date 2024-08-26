// A datagram socket server demo

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

#define PORT "4950"
#define MAX_LEN 50

int main()
{
    int sockfd;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE;    // to fill my ip address
    int rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (rv != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("server: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server : bind");
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "server : failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("server: to recvfrom...\n");
    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof client_addr;
    char str[INET6_ADDRSTRLEN];
    char buf[MAX_LEN];
    int numbytes = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&client_addr, &sin_size);
    if (numbytes == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    void *src;
    if (client_addr.ss_family == AF_INET)
        src = &(((struct sockaddr_in *)&client_addr)->sin_addr);
    else if (client_addr.ss_family == AF_INET6)
        src = &(((struct sockaddr_in6 *)&client_addr)->sin6_addr);
    inet_ntop(client_addr.ss_family, src, str, sizeof str);
    printf("Received packet from : %s\n", str);
    printf("Packet length : %d and Contents : '%s'\n", numbytes, buf);

    // char *msg = "Thanks for packet";
    // int sntbytes = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&client_addr, sin_size);
    // if (sntbytes == -1)
    // {
    //     perror("sendto");
    //     exit(1);
    // }
    // printf("Sent reply successfully\n");

    close(sockfd);
    printf("Bye\n");
    return 0;
}