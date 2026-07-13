#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#define MAX_PACKETS 1000000
#define PACKET_TIME_US 320 // 32 bits @ 100 kbps = 320 microseconds

// Precise sleep function using POSIX nanosleep
void sleep_us(long microseconds) {
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

// Function to verify if a 32-bit word has Odd Parity
int verify_odd_parity(uint32_t word) {
    uint32_t count = 0;
    // Count total number of set bits (1s) across all 32 bits
    for (int i = 0; i < 32; i++) {
        if ((word >> i) & 1) {
            count++;
        }
    }
    // Return 1 if odd (valid), 0 if even (invalid)
    return (count % 2 != 0);
}

// Helper to reverse bits for the Label (ARINC 429 Labels are usually read in Octal, reversed)
uint8_t reverse_bits(uint8_t num) {
    uint8_t count = 8;
    uint8_t reverse_num = 0;
    while (count--) {
        reverse_num <<= 1;
        reverse_num |= (num & 1);
        num >>= 1;
    }
    return reverse_num;
}

int main() {
    uint32_t packet_array[MAX_PACKETS];
    int packet_count = 0;

    // 1. Open the file
    FILE *file = fopen("altitude-hex.txt", "r");
    if (file == NULL) {
        perror("Error opening altitude-hex.txt");
        return EXIT_FAILURE;
    }

    // 2. Read hex values into the memory array
    while (packet_count < MAX_PACKETS && fscanf(file, "%x", &packet_array[packet_count]) == 1) {
        packet_count++;
    }
    fclose(file);

    if (packet_count == 0) {
        printf("No valid data found in altitude.txt\n");
        return EXIT_FAILURE;
    }

    printf("Loaded %d packets. Streaming and decoding at 100 kbps...\n", packet_count);
    printf("---------------------------------------------------------------------------\n");

    // 3. Continuously stream and decode packets at 100 kbps
    int i = 0;
    while (1) {
        uint32_t raw_word = packet_array[i];

        // Extract bit fields using masks and shifts
        uint8_t raw_label = raw_word & 0xFF;                 // Bits 1-8
        uint8_t sdi       = (raw_word >> 8) & 0x03;          // Bits 9-10
        uint32_t data     = (raw_word >> 10) & 0x3FFFF;      // Bits 11-28
        uint8_t ssm       = (raw_word >> 28) & 0x03;         // Bits 30-31
        int parity_valid = verify_odd_parity(raw_word);

        // ARINC 429 convention displays labels in Octal with reversed bit transmission
        uint8_t octal_label = reverse_bits(raw_label);

        // Print decoded breakdown
        printf("Raw Hex 0x%08X | Label %03o   |  Data(Dec) %-8u |  SDI %-5u | SSM %-6u %s\n", 
                raw_word, octal_label, data, sdi, ssm, 
               parity_valid ? "" : "[PARITY ERROR]");
        
        // Flush standard output immediately to maintain timing accuracy
        fflush(stdout); 

        // Throttle transmission to match 100 kbps
        sleep_us(PACKET_TIME_US);

        // Loop back to the beginning of the array
        i = (i + 1) % packet_count;
    }

    return EXIT_SUCCESS;
}
