#ifndef UDP_SENDER_H
#define UDP_SENDER_H

#include <stdint.h>
#include <netinet/in.h>

typedef struct {
    int sock_fd;
    struct sockaddr_in server_addr;
} udp_sender_t;

/**
 * @brief Initializes a UDP socket for sending data.
 */
int udp_sender_init(udp_sender_t *sender, const char *target_ip, int target_port);

/**
 * @brief Sends a 32-bit word over the configured UDP socket (handles network byte order).
 */
int udp_sender_send(udp_sender_t *sender, uint32_t data);

/**
 * @brief Closes the UDP socket.
 */
void udp_sender_close(udp_sender_t *sender);

#endif // UDP_SENDER_H