#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Function to reverse the 8-bit ARINC label (MSB to LSB transmission rule)
uint8_t reverse_label(uint8_t label) {
    uint8_t rev = 0;
    for (int i = 0; i < 8; i++) {
        rev |= ((label >> i) & 1) << (7 - i);
    }
    return rev;
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

int main() {
    FILE *file = fopen("altitude-hex.txt", "r");
    if (file == NULL) {
        perror("Error opening altitude-hex.txt");
        return 1;
    }

    uint32_t arinc_word;
    int packet_count = 0;


    // Read hex values line by line (handles both "0x12345678" and "12345678" formats)
    while (fscanf(file, "%x", &arinc_word) == 1) {
        packet_count++;

        // 1. Extract Label: Bits 1-8 (Lowest 8 bits)
        uint8_t raw_label = arinc_word & 0xFF;
        uint8_t octal_label = reverse_label(raw_label);

        // 2. Extract SDI: Bits 9-10
        uint8_t sdi = (arinc_word >> 8) & 0x03;

        // 3. Extract Data Payload: Bits 11-28 (18 bits)
        uint32_t data = (arinc_word >> 10) & 0x3FFFF;

        // 4. Extract SSM: Bits 29-30
        uint8_t ssm = (arinc_word >> 28) & 0x03;

        // 5. Verify Parity: Bit 32
        int parity_valid = verify_odd_parity(arinc_word);

        // Print extracted fields
        // Label is printed in standard 3-digit octal format (%03o)
        printf("Packet %-5d | Raw Hex 0x%08X | Label %03o   |  Data(Dec) %-8u |  SDI %-5u | SSM %-6u %s\n", 
               packet_count, arinc_word, octal_label, data, sdi, ssm, 
               parity_valid ? "" : "[PARITY ERROR]");
    }

    fclose(file);
    printf("\nFinished parsing %d packets.\n", packet_count);
    return 0;
}
