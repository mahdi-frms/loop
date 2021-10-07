#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <server.h>
#include <loop.h>

#define POOL_ACCEPT 1
#define POOL_READLINE 2

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

// void handle_client(void *arg, int *evl_pipe)
// {
//     int client = *(int *)arg;
//     while (1)
//     {
//         if (poll_dual(evl_pipe[0], client) == 0)
//         {
//             // event loop
//             break;
//         }
//         else
//         {
//             // client
//             char *string = server_recieve(client);
//             if (string == NULL || strcmp(string, "") == 0 || strcmp(string, "\n") == 0)
//             {
//                 break;
//             }
//             string = reverse(string);
//             if (server_send(client, string, strlen(string)) == -1)
//             {
//                 break;
//             }
//         }
//     }
//     close(client);
//     free(arg);
// }

void on_line(char *line)
{
    printf("line=%s", line);
}

void on_client(int client)
{
    printf("new client\n");
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

    evloop_t loop;
    evloop_initialize(&loop, threads);
    evloop_do_readline(&loop, on_line);
    evloop_do_accpet_client(&loop, &server, on_client);
    evloop_main_loop(&loop);
    evloop_destroy(&loop);
}