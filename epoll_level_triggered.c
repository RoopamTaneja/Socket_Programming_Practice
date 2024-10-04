// Example of using level triggered epoll instance

#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>

#define MAX_EVENTS 5
#define READ_SIZE 10

int main()
{
    int epoll_fd = epoll_create1(0); // create an epoll instance
    if (epoll_fd == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor\n");
        return 1;
    }
    int fd_to_inspect = STDIN_FILENO;
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd_to_inspect;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_to_inspect, &event))
    {
        fprintf(stderr, "Failed to add file descriptor\n");
        close(epoll_fd);
        return 1;
    }

    size_t bytes_read;
    char read_buffer[READ_SIZE + 1]; // for storing data read
    int event_cnt;
    struct epoll_event events[MAX_EVENTS]; // buffer to store triggered events
    int running = 1;
    while (running)
    {
        printf("\nPolling for input...\n");
        event_cnt = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000); // wait on epoll for 30s at a time
        printf("%d ready events\n", event_cnt);
        for (int i = 0; i < event_cnt; i++)
        {
            printf(" -- Reading fd '%d' -- \n", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
            read_buffer[bytes_read] = '\0';
            printf("Read : %s\n", read_buffer);
            if (!strncmp(read_buffer, "stop\n", 5))
                running = 0;
        }
    }
    if (close(epoll_fd))
    {
        fprintf(stderr, "Failed to close epoll file descriptor\n");
        return 1;
    }
    return 0;
}