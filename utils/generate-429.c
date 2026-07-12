
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Function to calculate Odd Parity for a 32-bit ARINC word
uint32_t calculate_odd_parity(uint32_t word) {
    uint32_t count = 0;
    // Count bits from 1 to 31 (bit 32 is the parity bit itself)
    for (int i = 0; i < 31; i++) {
        if ((word >> i) & 1) {
            count++;
        }
    }
    // If count is even, set bit 32 (MSB) to 1 to make total bits odd
    if (count % 2 == 0) {
        word |= (1U << 31);
    }
    return word;
}

int main() {
    FILE *file = fopen("altitude.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // ARINC 429 Label for Altitude is typically 203 (Octal)
    // 203 Octal = 10000011 Binary. 
    // In ARINC 429, the label bits are transmitted flipped (MSB to LSB).
    // Flipped 10000011 becomes 11000001 (0xC1 Hex).
    uint32_t label = 0xC1; 

    for (int i = 0; i < 1000000; i++) {
        uint32_t arinc_word = 0;

        // 1. Insert Label (Bits 1-8)
        arinc_word |= label;

        // 2. Insert SDI (Bits 9-10) -> Setting to 00 (System 1)
        uint32_t sdi = 0x0;
        arinc_word |= (sdi << 8);

        // 3. Generate Random Data (Bits 11-28 -> 18 bits)
        // Max value for 18 bits is 262,143 (represents altitude, e.g., in feet)
        uint32_t data = rand() % 262144; 
        arinc_word |= (data << 10);

        // 4. Insert SSM (Bits 29-30) -> Setting to 00 (Normal Operation)
        uint32_t ssm = 0x0;
        arinc_word |= (ssm << 28);

        // 5. Calculate and insert Odd Parity (Bit 32)
        arinc_word = calculate_odd_parity(arinc_word);

        // Write the resulting 32-bit word as an 8-character Hex string
        fprintf(file, "0x%08X\n", arinc_word);
    }

    fclose(file);
    printf("Successfully generated 10,000 ARINC 429 packets in altitude.txt\n");
    return 0;
}
