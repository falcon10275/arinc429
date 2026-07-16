#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "arinc_429.h"

uint8_t reverse_label_bits(uint8_t label) {
    uint8_t rev = 0;
    for (int i = 0; i < 8; i++) {
        rev |= ((label >> i) & 1) << (7 - i);
    }
    return rev;
}

uint32_t set_odd_parity(uint32_t word) {
    uint32_t temp = word & 0x7FFFFFFF; // Clear bit 32
    uint32_t count = 0;
    for (int i = 0; i < 31; i++) {
        if ((temp >> i) & 1) {
            count++;
        }
    }
    // If the count of 1s is even, we set the parity bit (Bit 32) to 1 to make it odd
    if (count % 2 == 0) {
        temp |= (1U << 31);
    }
    return temp;
}

void arinc429_set_label(arinc429_word_t *word, uint32_t label) {
    word->fields.label = label & 0xFF;       // 8 bits max
}

void arinc429_set_sdi(arinc429_word_t *word, uint32_t sdi) {
    word->fields.sdi = sdi & 0x03;           // 2 bits max
}

void arinc429_set_data(arinc429_word_t *word, uint32_t data) {
    word->fields.data = data & 0x7FFFF;      // 19 bits max
}

void arinc429_set_ssm(arinc429_word_t *word, uint32_t ssm) {
    word->fields.ssm = ssm & 0x03;           // 2 bits max
}

void arinc429_set_parity(arinc429_word_t *word, uint32_t parity) {
    word->fields.parity = parity & 0x01;     // 1 bit max
}

uint32_t encode_altitude(double altitude_ft, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (203 Octal = 0x83 Hex)
    uint8_t rev_label = reverse_label_bits(0x83);
    word |= (rev_label & 0xFF);

    // 2. SDI (Bits 9-10)
    word |= ((sdi & 0x03) << 8);

    // Clamp value to safe boundaries
    if (altitude_ft > 131072.0) altitude_ft = 131072.0;
    if (altitude_ft < -1000.0) altitude_ft = -1000.0;

    // Determine Sign (Bit 29)
    uint32_t sign_bit = 0;
    if (altitude_ft < 0) {
        sign_bit = 1;
        altitude_ft = -altitude_ft; // Work with absolute value for encoding
    }

    // Convert to binary data (LSB = 1.0 ft, mapped to Bits 12-28)
    uint32_t data_val = (uint32_t)round(altitude_ft);
    word |= ((data_val & 0x1FFFF) << 11); // 17 bits of data (Bits 12-28)

    // Apply Sign Bit (Bit 29)
    word |= (sign_bit << 28);

    // 3. SSM (Bits 30-31)
    word |= ((ssm & 0x03) << 29);

    // 4. Odd Parity (Bit 32)
    return set_odd_parity(word);
}


uint32_t encode_bank_angle(double angle_deg, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (325 Octal = 0xD5 Hex)
    uint8_t rev_label = reverse_label_bits(0xD5);
    word |= (rev_label & 0xFF);

    // 2. SDI (Bits 9-10)
    word |= ((sdi & 0x03) << 8);

    // Clamp value
    if (angle_deg > 180.0) angle_deg = 180.0;
    if (angle_deg < -180.0) angle_deg = -180.0;

    // Determine Sign (Bit 29)
    uint32_t sign_bit = 0;
    if (angle_deg < 0) {
        sign_bit = 1;
        angle_deg = -angle_deg;
    }

    // Convert to Binary: scale factor is (2^15) / 180.0
    uint32_t data_val = (uint32_t)round(angle_deg * (32768.0 / 180.0));
    word |= ((data_val & 0x7FFF) << 13); // 15 bits of data (Bits 14-28)

    // Apply Sign Bit (Bit 29)
    word |= (sign_bit << 28);

    // 3. SSM (Bits 30-31)
    word |= ((ssm & 0x03) << 29);

    // 4. Odd Parity (Bit 32)
    return set_odd_parity(word);
}

uint32_t encode_engine_rpm(double rpm_percent, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (341 Octal = 0xE1 Hex)
    uint8_t rev_label = reverse_label_bits(0xE1);
    word |= (rev_label & 0xFF);

    // 2. SDI (Bits 9-10)
    word |= ((sdi & 0x03) << 8);

    // Clamp value
    if (rpm_percent > 110.0) rpm_percent = 110.0;
    if (rpm_percent < 0.0) rpm_percent = 0.0;

    // Convert to Binary: Scale factor uses Bit 28 = 64%, mapping to 18 bits (Bits 11-28)
    // Bit 29 is often part of the data field or unused sign here since RPM is always positive.
    uint32_t data_val = (uint32_t)round(rpm_percent * (262144.0 / 128.0));
    word |= ((data_val & 0x3FFFF) << 10); // 18 bits of data (Bits 11-28)

    // 3. SSM (Bits 29-30 or 30-31 depending on system layout, using Bits 29-30 standard for BNR)
    word |= ((ssm & 0x03) << 29);

    // 4. Odd Parity (Bit 32)
    return set_odd_parity(word);
}