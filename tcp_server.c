// A stream socket server demo

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

// signal handler function for SIGCHLD
// Defines a handler that reaps terminated child processes to prevent
// zombie processes from accumulating. It uses waitpid in a loop to
// reap all child processes that have finished.
void sigchld_handler(int s)
{
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

volatile sig_atomic_t server_running = 1;

// signal handler function for graceful shutdown on Ctrl-C
void shutdown_handler(int sig)
{
    server_running = 0;
}

#define PORT "3490"
#define BACKLOG 10 // pending connections queue can hold
#define MAX_SIZE 50

int main()
{
    int sockfd, new_fd; // sockfd -> listening socket; new_fd -> connection socket
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;     // to fill my ip address

    int rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (rv != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    struct addrinfo *p;
    int yes = 1;
    for (p = servinfo; p != NULL; p = p->ai_next) // go through all structs in list
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) // avoid address in use errors
        {
            perror("setsockopt");
            // can free heap memory, although not needed since program is exiting
            exit(1);
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
        exit(1);
    }

    freeaddrinfo(servinfo);


    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    // configures the signal handler for SIGCHLD using sigaction after setting up right flags
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    // This setup is typically used in server programs or other long-running processes
    // that fork child processes. Handling SIGCHLD properly is crucial for managing
    // child processes and avoiding issues with zombie processes.

    // setup signal handler for graceful shutdown
    struct sigaction sa2;
    sa2.sa_handler = shutdown_handler;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    if (sigaction(SIGINT, &sa2, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof client_addr;
    char str[INET6_ADDRSTRLEN];
    int client_count = 0;
    while (server_running) // main server loop
    {
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1)
        {
            if (errno == EINTR && !server_running)
                break;
            perror("accept");
            continue;
        }
        void *src;
        if (client_addr.ss_family == AF_INET)
            src = &(((struct sockaddr_in *)&client_addr)->sin_addr);
        else if (client_addr.ss_family == AF_INET6)
            src = &(((struct sockaddr_in6 *)&client_addr)->sin6_addr);
        inet_ntop(client_addr.ss_family, src, str, sizeof str);
        printf("Connected to client : %s\n", str);
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            close(new_fd);
            continue;
        }
        client_count++;
        if (pid == 0) // child
        {
            close(sockfd); // child doesn't need listening socket
            char msg[MAX_SIZE];
            snprintf(msg, sizeof(msg), "Hello, client %d", client_count);
            if (send(new_fd, msg, strlen(msg), 0) == -1)
                perror("send");
            close(new_fd);
            exit(0); // job of child is over, it can exit
        }
        close(new_fd); // listener parent does not need this socket
    }

    close(sockfd);
    printf("\nserver: shutting down...\n");
    return 0;
}