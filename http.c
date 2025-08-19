#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define SERVER_PORT     9191
#define MAX_CONNECTIONS 1
#define READ_BUF_SIZE   1024

static volatile sig_atomic_t stop;

void handle_sigint(int sig)
{
    (void)sig;
    stop = 1;
}

static int init_server()
{
    int sfd;
    struct sockaddr_in addr = { 0 };
    int opt                 = 1;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        goto out;
    }

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        goto out;
    }

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        goto out;
    }

    if (listen(sfd, MAX_CONNECTIONS) == -1) {
        perror("listen");
        goto out;
    }

    puts("server started");
    printf("listening on: 127.0.0.1:%d\n\n", ntohs(addr.sin_port));
    return sfd;

out:
    close(sfd);
    return -1;
}

static int accept_client(int sfd)
{
    int cfd;
    struct sockaddr_in addr = { 0 };
    socklen_t len           = sizeof(addr);

    cfd = accept(sfd, (struct sockaddr *)&addr, &len);
    if (cfd == -1) {
        perror("accept");
        return -1;
    }

    /* inet_ntoa is MT-Safe (locale); see man 7 attributes */
    printf("client connected: %s:%d\n", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));

    return cfd;
}

static void serve_client(int cfd)
{
    for (;;) {
        char buf[READ_BUF_SIZE];

        ssize_t nread = read(cfd, buf, sizeof(buf));
        if (nread <= 0) {
            if (nread < 0)
                perror("read");
            break;
        }

        ssize_t written = 0;
        while (written < nread) {
            size_t rem  = (size_t)(nread - written);
            ssize_t ret = write(cfd, buf + written, rem);

            if (ret == -1) {
                perror("write");
                break;
            }

            written += ret;
        }
    }

    close(cfd);
    puts("client disconnected\n");
}

int main(void)
{
    signal(SIGINT, handle_sigint);

    int server_fd = init_server();
    if (server_fd < 0)
        return 1;

    while (!stop) {
        int client_fd = accept_client(server_fd);
        if (client_fd < 0)
            continue;

        serve_client(client_fd);
    }

    close(server_fd);
    puts("server stopped");
    return 0;
}
