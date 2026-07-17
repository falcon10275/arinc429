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

uint8_t get_decoded_label(uint32_t word) {
    uint8_t raw_label = word & 0xFF;
    uint8_t rev = 0;
    for (int i = 0; i < 8; i++) {
        rev |= ((raw_label >> i) & 1) << (7 - i);
    }
    return rev;
}

uint8_t extract_arinc_label(uint32_t word) {
    uint8_t raw_label = (uint8_t)(word & 0xFF);
    uint8_t decoded_label = 0;
    for (int i = 0; i < 8; i++) {
        decoded_label |= ((raw_label >> i) & 1) << (7 - i);
    }
    return decoded_label;
}

bool verify_parity(uint32_t word) {
    uint32_t count = 0;
    for (int i = 0; i < 32; i++) {
        if ((word >> i) & 1) {
            count++;
        }
    }
    return (count % 2 != 0);
}

uint32_t encode_altitude(double altitude_ft, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (203 Octal = 0x83 Hex)
    uint8_t rev_label = reverse_label_bits(LABEL_ALTITUDE);
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

DecodedArincWord decode_altitude(uint32_t word) {
    DecodedArincWord result;
    
    result.parity_valid = verify_parity(word);
    result.label = get_decoded_label(word);
    result.sdi = (word >> 8) & 0x03;
    result.ssm = (word >> 29) & 0x03;

    // Extract raw data field: 17 bits (Bits 12-28)
    uint32_t raw_data = (word >> 11) & 0x1FFFF;
    
    // Extract sign bit: Bit 29 (1 = Negative, 0 = Positive)
    uint32_t sign_bit = (word >> 28) & 0x01;

    double alt_val = (double)raw_data; // Scale is 1.0 LSB = 1 ft
    if (sign_bit == 1) {
        alt_val = -alt_val;
    }
    
    result.value = alt_val;
    return result;
}



uint32_t encode_bank_angle(double angle_deg, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (325 Octal = 0xD5 Hex)
    uint8_t rev_label = reverse_label_bits(LABEL_BANK_ANGLE);
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

DecodedArincWord decode_bank_angle(uint32_t word) {
    DecodedArincWord result;
    
    result.parity_valid = verify_parity(word);
    result.label = get_decoded_label(word);
    result.sdi = (word >> 8) & 0x03;
    result.ssm = (word >> 29) & 0x03;

    // Extract raw data field: 15 bits (Bits 14-28)
    uint32_t raw_data = (word >> 13) & 0x7FFF;
    
    // Extract sign bit: Bit 29 (1 = Left/Minus, 0 = Right/Plus)
    uint32_t sign_bit = (word >> 28) & 0x01;

    // Scale back: (raw_data * 180.0) / 2^15
    double angle_val = ((double)raw_data * 180.0) / 32768.0;
    if (sign_bit == 1) {
        angle_val = -angle_val;
    }
    
    result.value = angle_val;
    return result;
}

uint32_t encode_engine_rpm(double rpm_percent, uint8_t sdi, uint8_t ssm) {
    uint32_t word = 0;
    
    // 1. Label (341 Octal = 0xE1 Hex)
    uint8_t rev_label = reverse_label_bits(LABEL_ENGINE_RPM);
    word |= (rev_label & 0xFF);

    // 2. SDI (Bits 9-10)
    word |= ((sdi & 0x03) << 8);

    // Convert to Binary: Scale factor uses Bit 28 = 64%, mapping to 18 bits (Bits 11-28)
    // Bit 29 is often part of the data field or unused sign here since RPM is always positive.
    uint32_t data_val = (uint32_t)round(rpm_percent);
    word |= ((data_val & 0x3FFFF) << 10); // 18 bits of data (Bits 11-28)

    // 3. SSM (Bits 29-30 or 30-31 depending on system layout, using Bits 29-30 standard for BNR)
    word |= ((ssm & 0x03) << 29);

    // 4. Odd Parity (Bit 32)
    return set_odd_parity(word);
}

DecodedArincWord decode_engine_rpm(uint32_t word) {
    DecodedArincWord result;
    
    result.parity_valid = verify_parity(word);
    result.label = get_decoded_label(word);
    result.sdi = (word >> 8) & 0x03;
    result.ssm = (word >> 29) & 0x03;

    // Extract raw data field: 18 bits (Bits 11-28)
    uint32_t raw_data = (word >> 10) & 0x3FFFF;

    result.value = (double)raw_data;
    
    return result;
}