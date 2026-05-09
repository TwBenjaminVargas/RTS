#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "protocol.h"

static void send_packet(int fd, Command cmd, const char *data) {
    Packet packet;
    memset(&packet, 0, sizeof(packet));

    packet.cmd = cmd;

    if (data != NULL) {
        packet.length = (uint16_t)strlen(data);
        strncpy(packet.data, data, BUFFER_SIZE - 1);
    }

    ssize_t bytes_written = write(fd, &packet, sizeof(Packet));
    if (bytes_written == -1) {
        perror("Writing error");
    }
}

int main(int argc, char *argv[]) {
    
     if (argc != 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(argv[1], "on") != 0 && strcmp(argv[1], "off") != 0 && strcmp(argv[1], "status") != 0) {
        fprintf(stderr, "Invalid command. Use 'on', 'off', or 'status'.\n");
        exit(EXIT_FAILURE);
    }
    
    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("Socket connection error"); exit(EXIT_FAILURE); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect"); close(fd); exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    if (strcmp(argv[1], "on") == 0) {
        send_packet(fd, CMD_ON, NULL);
    } else if (strcmp(argv[1], "off") == 0) {
        send_packet(fd, CMD_OFF, NULL);
    } else if (strcmp(argv[1], "status") == 0) {
        send_packet(fd, CMD_STATUS, NULL);
    }

    Packet ack;
    ssize_t bytes_read = read(fd, &ack, sizeof(Packet));
    if (bytes_read == -1) {
        perror("Ack read error");
    } else if (ack.cmd == ACK) {
        printf("Server response: %s\n", ack.data);
    }


    close(fd);
    return 0;
}