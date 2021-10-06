#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pool.h>
#include <server.h>
#include <poll.h>

#define POOL_ACCEPT 1
#define POOL_READLINE 2

int poll_dual(int fd1, int fd2)
{
    struct pollfd fds[2];
    fds[0].fd = fd1;
    fds[0].events = POLLIN;
    fds[1].fd = fd2;
    fds[1].events = POLLIN;
    poll(fds, 2, -1);
    return fds[1].revents == POLLIN;
}

size_t poll_array(int *fds, size_t len)
{
    struct pollfd pfds[len];
    for (size_t i = 0; i < len; i++)
    {
        pfds[i].fd = fds[i];
        pfds[i].events = POLLIN;
    }
    poll(pfds, len, -1);
    for (size_t i = 0; i < len; i++)
    {
        if (pfds[i].revents == POLLIN)
        {
            return i;
        }
    }
    return 0;
}

char *reverse(const char *string)
{
    size_t size = strlen(string);
    char *rev = malloc(size + 1);
    for (size_t i = 0; i < size - 1; i++)
    {
        rev[i] = string[size - 2 - i];
    }
    rev[size - 1] = '\n';
    rev[size] = '\0';
    return rev;
}

void handle_client(void *arg, int *evl_pipe)
{
    int client = *(int *)arg;
    while (1)
    {
        if (poll_dual(evl_pipe[0], client) == 0)
        {
            // event loop
            break;
        }
        else
        {
            // client
            char *string = server_recieve(client);
            if (string == NULL || strcmp(string, "") == 0 || strcmp(string, "\n") == 0)
            {
                break;
            }
            string = reverse(string);
            if (server_send(client, string, strlen(string)) == -1)
            {
                break;
            }
        }
    }
    close(client);
    free(arg);
}

void accept_client(void *arg, int *evl_pipe)
{
    server_t *server = arg;
    int *client;
    while (1)
    {
        if (poll_dual(evl_pipe[0], server->fd) == 0)
        {
            break;
        }
        else
        {
            client = malloc(sizeof(int));
            *client = server_accept(server);
            write(evl_pipe[1], &client, sizeof(int *));
        }
    }
}

void get_line(void *_, int *evl_pipe)
{
    char *line = NULL;
    size_t alloc;
    while (1)
    {
        if (poll_dual(evl_pipe[0], STDIN_FILENO) == 0)
        {
            break;
        }
        else
        {
            getline(&line, &alloc, stdin);
            write(evl_pipe[1], &line, sizeof(char *));
        }
    }
}

int main(int argc, char **argv)
{
    int port = 8080;
    int threads = 8;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    if (argc > 2)
    {
        threads = atoi(argv[2]);
    }
    server_t server = server_create(port);
    threadpool_t pool;
    pool_initialize(&pool, threads);
    if (server.fd == -1)
    {
        perror("Error: server initialization failed\n");
        return 1;
    }
    if (server_listen(&server) == -1)
    {
        perror("Error: server failed to listen\n");
        return 1;
    }
    printf("listening on port %d...\n", port);
    int thread_accept[2];
    int thread_readline[2];
    pool_execute(&pool, accept_client, &server, thread_accept);
    pool_execute(&pool, get_line, NULL, thread_readline);
    while (1)
    {
        int poll_rsl = poll_dual(thread_accept[0], thread_readline[0]);
        if (poll_rsl == 0)
        {
            int *client = malloc(sizeof(int));
            int fd[2];
            read(thread_accept[0], &client, sizeof(int *));
            pool_execute(&pool, handle_client, client, fd);
        }
        else
        {
            char *input;
            read(thread_readline[0], &input, sizeof(char *));
            if (strcmp(input, "fin\n") == 0)
            {
                break;
            }
            else
            {
                printf("shell>>> %s\n", input);
            }
        }
    }
    char term = '\n';
    write(thread_accept[1], &term, sizeof(char));
    write(thread_readline[1], &term, sizeof(char));
    pool_destroy(&pool);
    printf("ended:(\n");
    return 0;
}