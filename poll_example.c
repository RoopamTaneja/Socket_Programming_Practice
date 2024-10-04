#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#define MAX_FDS 1
#define READ_SIZE 10

int main()
{
    struct pollfd pfds[MAX_FDS];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;

    size_t bytes_read;
    char read_buffer[READ_SIZE + 1];

    printf("Hit RETURN or wait 30 seconds for timeout\n");

    int num_events = poll(pfds, MAX_FDS, 30000);
    if (num_events > 0)
    {
        for (int i = 0; i < MAX_FDS; i++)
        {
            int pollin_bool = pfds[i].revents & POLLIN;
            if (pollin_bool)
            {
                printf("File descriptor %d is ready to read\n", pfds[i].fd);
                printf(" -- Reading fd '%d' -- \n", pfds[i].fd);
                bytes_read = read(pfds[i].fd, read_buffer, READ_SIZE);
                if (bytes_read < 0)
                {
                    perror("Error reading from fd");
                }
                else
                {
                    read_buffer[bytes_read] = '\0'; // Null-terminate the string
                    printf("Read: %s\n", read_buffer);
                }
            }
            else
            {
                printf("Unexpected event occurred: %d\n", pfds[i].revents);
            }
        }
    }
    else if (num_events == 0)
    {
        printf("Poll timed out!\n");
    }
    else
    {
        perror("Error in polling\n");
    }
    return 0;
}