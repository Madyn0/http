#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT    9191
#define BACKLOG 1

int main()
{
    int listen_fd, client_fd;
    struct sockaddr_in listen_addr, client_addr;

    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_port        = htons(PORT);
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(listen_fd, (struct sockaddr *)&listen_addr,
             sizeof(listen_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("listening on: %s:%d\n", inet_ntoa(listen_addr.sin_addr),
           ntohs(listen_addr.sin_port));

    socklen_t client_addr_len = sizeof(client_addr);
    client_fd =
        accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        perror("accept");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    write(client_fd, "foobar\n", 7);

    close(client_fd);
    puts("client disconnected");

    close(listen_fd);
    puts("server stopped");

    return EXIT_SUCCESS;
}
