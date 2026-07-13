#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999
#define INTERVAL_US_429_HIGH 320 // 320 microseconds for ~100kbps
#define INTERVAL_US_429_LOW 2560 // 2560 microseconds for 12.5kbps

uint32_t generate_random_arinc() {
    // Randomize fields: 8-bit label, 2-bit SDI, 19-bit data, 2-bit SSM, 1-bit parity
    uint32_t label = rand() & 0xFF;
    uint32_t sdi   = rand() & 0x03;
    uint32_t data  = rand() & 0x7FFFF;
    uint32_t ssm   = rand() & 0x03;
    uint32_t parity= rand() & 0x01;

    return (parity << 31) | (ssm << 29) | (data << 10) | (sdi << 8) | label;
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    srand(time(NULL));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TARGET_PORT);
    inet_pton(AF_INET, TARGET_IP, &server_addr.sin_addr);

    printf("Starting stream at 100kbps...\n");

    while (1) {
        uint32_t packet = htonl(generate_random_arinc());
        
        sendto(sock_fd, &packet, sizeof(packet), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));
        
        // Sleep for 320 microseconds to maintain rate
        usleep(INTERVAL_US_429_HIGH);
    }

    close(sock_fd);
    return 0;
}