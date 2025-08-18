#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9191

void cleanup(int sock);

int main()
{
    struct sockaddr_in host;

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    puts("socket created");

    host.sin_family      = AF_INET;
    host.sin_port        = htons(PORT);
    host.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock, (struct sockaddr *)&host, sizeof(host)) == -1) {
        perror("bind");
        cleanup(sock);
        exit(EXIT_FAILURE);
    }
    puts("bind created");

    cleanup(sock);
    return 0;
}

void cleanup(int sock)
{
    if (close(sock) == -1)
        perror("close");

    puts("cleaned up");
}
