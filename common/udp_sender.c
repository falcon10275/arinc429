#include "udp_sender.h"
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int udp_sender_init(udp_sender_t *sender, const char *target_ip, int target_port) {
    sender->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender->sock_fd < 0) {
        return -1;
    }

    memset(&sender->server_addr, 0, sizeof(sender->server_addr));
    sender->server_addr.sin_family = AF_INET;
    sender->server_addr.sin_port = htons(target_port);
    
    if (inet_pton(AF_INET, target_ip, &sender->server_addr.sin_addr) <= 0) {
        close(sender->sock_fd);
        return -1;
    }

    return 0;
}

int udp_sender_send(udp_sender_t *sender, uint32_t data) {
    uint32_t packet = htonl(data); // Convert to network byte order
    return sendto(sender->sock_fd, &packet, sizeof(packet), 0,
                  (struct sockaddr *)&sender->server_addr, sizeof(sender->server_addr));
}

void udp_sender_close(udp_sender_t *sender) {
    if (sender->sock_fd >= 0) {
        close(sender->sock_fd);
        sender->sock_fd = -1;
    }
}