#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999

// Helper function to mock an ARINC 429 32-bit word
// ARINC 429 Word Format: 
// [Bit 31: Parity] [Bits 30-29: SSM] [Bits 28-9: Data] [Bits 8-7: SDI] [Bits 6-0: Label]
// Note: Bit numbering can vary (1-32 vs 0-31 depending on documentation)
uint32_t create_arinc429_word(uint8_t label, uint8_t sdi, uint32_t data, uint8_t ssm, uint8_t parity) {
    uint32_t word = 0;
    
    word |= (label & 0xFF);          // Bits 0-7 (8 bits)
    word |= ((sdi & 0x03) << 8);     // Bits 8-9 (2 bits)
    word |= ((data & 0x7FFFF) << 10); // Bits 10-28 (19 bits)
    word |= ((ssm & 0x03) << 29);    // Bits 29-30 (2 bits)
    word |= ((parity & 0x01) << 31); // Bit 31 (1 bit)
    
    return word;
}

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    uint32_t arinc_word;
    uint32_t network_ordered_word;

    // 1. Create a UDP socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configure the target destination (localhost:9999)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TARGET_PORT);
    
    if (inet_pton(AF_INET, TARGET_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Generate a sample ARINC 429 word
    // Example parameters: Label=0376 (Octal), SDI=1, Data=0x1234, SSM=0, Parity=1
    arinc_word = create_arinc429_word(0xFD, 1, 0x1234, 0, 1);
    
    printf("Original ARINC 429 Word: 0x%08X\n", arinc_word);

    // 4. Convert to Network Byte Order (Big Endian) 
    // Standard practice for UDP payloads, though ensure your receiver expects this.
    network_ordered_word = htonl(arinc_word);

    // 5. Send the packet
    ssize_t bytes_sent = sendto(sock_fd, &network_ordered_word, sizeof(network_ordered_word), 0,
                                (const struct sockaddr *)&server_addr, sizeof(server_addr));
    
    if (bytes_sent < 0) {
        perror("Transmission failed");
    } else {
        printf("Successfully sent %zd bytes to %s:%d\n", bytes_sent, TARGET_IP, TARGET_PORT);
    }

    // 6. Clean up
    close(sock_fd);
    return 0;
}