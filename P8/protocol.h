#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/control_led.sock"
#define BUFFER_SIZE 256

typedef enum __attribute__((packed)) {
    ACK    = 0xFE,
    CMD_ON    = 0x01,
    CMD_OFF = 0x02,
    CMD_STATUS    = 0x03,
} Command;

typedef struct __attribute__((packed)) {
    Command  cmd;
    uint16_t length;
    char     data[BUFFER_SIZE];
} Packet;

#endif // PROTOCOL_H