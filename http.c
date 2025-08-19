#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define PORT      9191
#define BACKLOG   1
#define BUFF_SIZE 1024

static int init_server()
{
    int sfd;
    struct sockaddr_in addr = { 0 };
    int opt                 = 1;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        goto fail;
    }

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        goto fail;
    }

    if (listen(sfd, BACKLOG) == -1) {
        perror("listen");
        goto fail;
    }

    printf("listening on: 127.0.0.1:%d\n", ntohs(addr.sin_port));
    return sfd;

fail:
    close(sfd);
    return -1;
}

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

    listen_fd = init_server();

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
