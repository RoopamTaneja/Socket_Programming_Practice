// A datagram socket client demo
// Fixed to communicate on IPv4 so that no mismatch of 
// server and client working on different versions
// IPv4 bcoz IPv6 not working for some reason

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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: ./client.out hostname message\n");
        exit(1);
    }
    char *msg = argv[2];
    int sockfd;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    int rv = getaddrinfo(argv[1], PORT, &hints, &servinfo);
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
            perror("client: socket");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }

    int numbytes = sendto(sockfd, msg, strlen(msg), 0, p->ai_addr, p->ai_addrlen);
    if (numbytes == -1)
    {
        perror("sendto");
        exit(1);
    }
    freeaddrinfo(servinfo);
    printf("Sent message of %d bytes successfully\n", numbytes);

    // struct sockaddr_storage return_addr;
    // socklen_t sin_size = sizeof return_addr;
    // char buf[MAX_LEN];
    // numbytes = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&return_addr, &sin_size);
    // if (numbytes == -1)
    // {
    //     perror("recvfrom");
    //     exit(1);
    // }
    // buf[numbytes] = '\0';
    // printf("Packet received : '%s'\n", buf);

    close(sockfd);
    printf("Bye\n");
    return 0;
}