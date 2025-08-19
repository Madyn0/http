#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define PORT      9191
#define BACKLOG   1
#define BUFF_SIZE 1024

static void serve_client(int client_fd)
{
    for (;;) {
        char buff[BUFF_SIZE];
        ssize_t bytes_read, total_written;

        bytes_read = read(client_fd, buff, sizeof(buff));
        if (bytes_read <= 0) {
            if (bytes_read < 0)
                perror("read");
            break;
        }

        total_written = 0;
        while (total_written < bytes_read) {
            ssize_t just_written;

            just_written = write(client_fd, buff + total_written,
                                 bytes_read - total_written);
            if (just_written == -1) {
                perror("write");
                return;
            }

            total_written += just_written;
        }
    }

    if (close(client_fd) == -1)
        perror("close client_fd");
    puts("client disconnected");
}

int main(void)
{
    int listen_fd;
    struct sockaddr_in listen_addr = { 0 };

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) {
        perror("setsockopt");
        goto out_close;
    }

    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_port        = htons(PORT);
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(listen_fd, (struct sockaddr *)&listen_addr,
             sizeof(listen_addr)) == -1) {
        perror("bind");
        goto out_close;
    }

    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        goto out_close;
    }

    printf("listening on: 127.0.0.1:%d\n", ntohs(listen_addr.sin_port));

    for (;;) {
        struct sockaddr_in client_addr = { 0 };
        socklen_t client_addr_len      = sizeof(client_addr);
        int client_fd;

        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr,
                           &client_addr_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        char client_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_str,
                  sizeof(client_str));
        printf("client connected: %s:%d\n", client_str,
               ntohs(client_addr.sin_port));

        serve_client(client_fd);
    }

out_close:
    if (close(listen_fd) == -1)
        perror("close listen_fd");

    return 1;
}
