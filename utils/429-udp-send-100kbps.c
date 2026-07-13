#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999
#define INTERVAL_US_429_HIGH 3680000 // ~100kbps  ARINC 429 HIGH SPEED
#define INTERVAL_US_429_LOW 29440000 // ~12.5kbps ARINC 429 LOW SPEED

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

    //long total_duration_ns_high = 3680000;
    // 29,440,000 ns for 12.5 kbps (ARINC 429 Low Speed)
    //long total_duration_ns_low = 29440000;


    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    srand(time(NULL));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TARGET_PORT);
    inet_pton(AF_INET, TARGET_IP, &server_addr.sin_addr);

    printf("Starting 429 UDP stream ...\n");

    while (1) {

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        uint32_t packet = htonl(generate_random_arinc());
        
        sendto(sock_fd, &packet, sizeof(packet), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        clock_gettime(CLOCK_MONOTONIC, &end);
        
        long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    
        // Calculate remaining sleep time
        long sleep_time_ns = INTERVAL_US_429_HIGH - elapsed_ns;

        // Sleep if we are still ahead of schedule
        if (sleep_time_ns > 0) {
            struct timespec sleep_req = {0, sleep_time_ns};
            nanosleep(&sleep_req, NULL);
        }
    }

    close(sock_fd);
    return 0;
}