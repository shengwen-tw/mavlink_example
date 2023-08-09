#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int open_net(char *ip, int port_num)
{
    /* socket initialization */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Failed to create socket.\n");
        exit(1);
    }

    /* attempt to connect to the server */
    struct sockaddr_in serv_addr = {.sin_family = AF_INET,
                                    .sin_addr.s_addr = inet_addr(ip),
                                    .sin_port = htons(port_num)};
    if (connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
        printf("Failed to connect to the server.\n");
        exit(1);
    }

    return socket_fd;
}
