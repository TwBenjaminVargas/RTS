#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include <pigpio.h>
#include <sys/time.h>
#include <errno.h>

#include "protocol.h"

#define LED_PIN 18
#define CLIENTS_QUEUE_SIZE 5

typedef struct {
    pthread_mutex_t mutex;
    bool            status;
} LedState;


static LedState led_state = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .status = false
};

atomic_bool keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
}

void led_set(LedState *state, bool value) {
    pthread_mutex_lock(&state->mutex);
    state->status = value;
    gpioWrite(LED_PIN, value ? 1 : 0);
    pthread_mutex_unlock(&state->mutex);
}

bool led_get(LedState *state) {
    pthread_mutex_lock(&state->mutex);
    bool estado = state->status;
    pthread_mutex_unlock(&state->mutex);
    return estado;
}
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
        perror("Writing to client failed");
    }
}

void* client_manager(void* arg) {
    int client_fd = (int)(intptr_t)arg;
    ssize_t bytes_read;
    Packet packet;

    while ((bytes_read = read(client_fd, &packet, sizeof(Packet))) > 0)
    {
        switch (packet.cmd) {
            case CMD_ON:
                printf("Client %lu request: Turn LED on\n", (unsigned long)pthread_self());
                led_set(&led_state, true);
                break;
            case CMD_OFF:
                printf("Client %lu request: Turn LED off\n", (unsigned long)pthread_self());
                led_set(&led_state, false);
                break;
            case CMD_STATUS:
                printf("Client %lu request: Get LED status\n", (unsigned long)pthread_self());
                break;
            default:
                break;
            }
            bool status = led_get(&led_state);
            send_packet(client_fd, ACK, status ? "LED_OK: ON" : "LED_OK: OFF");
    }
    close(client_fd);
    printf("Client disconnected, thread ID: %lu\n", (unsigned long)pthread_self());
    return NULL;

}

int socket_init() {
    int server_fd;
    struct sockaddr_un addr;
    
    if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct timeval tv = {
        .tv_sec  = 1,
        .tv_usec = 0,
    };

    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if(bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        socket_cleanup(server_fd);
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }
    if(listen(server_fd, CLIENTS_QUEUE_SIZE) == -1) {
        socket_cleanup(server_fd);
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void socket_cleanup(int fd)
{
    unlink(SOCKET_PATH);
    close(fd);
    
}

void* socket_manager(void* arg) {

    int server_fd = socket_init(), client_fd;
    printf("Server is listening on %s\n", SOCKET_PATH);

    while (keep_running)
    {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
            
                continue;  // timeout, re-chequear keep_running
            }
            perror("Failed to accept client connection");
            if (errno == EMFILE || errno == ENFILE) {
                fprintf(stderr, "Fatal: too many open files\n");
                keep_running = 0;
            }
            continue;
        }
        
        launch_client_manager(client_fd);
    }

    socket_cleanup(server_fd);    
    return NULL;
}

void launch_client_manager(int client_fd) {
    
    pthread_t thread_id;


    if (pthread_create(&thread_id, NULL, client_manager, (void *)(intptr_t)client_fd) != 0) {
        perror("Failed to create client manager thread");
        close(client_fd);
        return;
        
    }
    printf("Client connected, thread ID: %lu\n", (unsigned long)thread_id);
    pthread_detach(thread_id);
}

int main(void) {
    pthread_t socket_thread;

    signal(SIGINT, handle_sigint);


    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        exit(EXIT_FAILURE);
    }

    gpioSetMode(LED_PIN, PI_OUTPUT);



    if (pthread_create(&socket_thread, NULL, socket_manager, NULL) != 0) {
        perror("Failed to create socket thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(socket_thread, NULL);
    gpioTerminate();

    return 0;
}