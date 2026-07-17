#ifndef ARINC_429_H
#define ARINC_429_H

#include <stdbool.h>
#include <stdint.h>

/*
ARINC 429 Word Format
An ARINC 429 word is 32 bits wide and is structured as follows:
•	Bits 1-8: Label (transmitted LSB first, meaning the label bits are reversed)
•	Bits 9-10: Source/Destination Identifier (SDI)
•	Bits 11-28: Data Field (binary representation, BNR)
•	Bits 29-30: Sign/Status Matrix (SSM) — e.g., 3 (0b11) for Normal Operation (NO)
•	Bit 31: Sign bit (for signed parameters, or part of data depending on the layout)
•	Bit 32: Parity bit (historically odd parity is standard)
*/

// ARINC 429 SSM Definitions for BNR (Binary) data
#define SSM_FAILURE_WARNING   0x0 // 00
#define SSM_NO_COMPUTED_DATA  0x1 // 01
#define SSM_FUNCTIONAL_TEST   0x2 // 10
#define SSM_NORMAL_OPERATION  0x3 // 11

// Octal Label Definitions (Note: 0 prefixes in C represent Octal literals)
#define LABEL_ALTITUDE   0203  // Octal 203 (Decimal 131)
#define LABEL_BANK_ANGLE 0325  // Octal 325 (Decimal 213)
#define LABEL_ENGINE_RPM 0341  // Octal 341 (Decimal 225)

typedef struct {
    uint8_t label;
    uint8_t sdi;
    uint8_t ssm;
    double value;
    bool parity_valid;
} DecodedArincWord;

typedef union {
    struct {
        uint32_t label  : 8;  // Bits 0-7
        uint32_t sdi    : 2;  // Bits 8-9
        uint32_t data   : 19; // Bits 10-28
        uint32_t ssm    : 2;  // Bits 29-30
        uint32_t parity : 1;  // Bit 31
    } fields;
    uint32_t raw;
} arinc429_word_t;

// Helper function to reverse 8-bit label 
// (ARINC 429 transmits Bit 1 as Label MSB, effectively reversing standard octal representation)
uint8_t reverse_label_bits(uint8_t label); 

// Helper function to reverse 8-bit label
uint8_t get_decoded_label(uint32_t word);

uint8_t extract_arinc_label(uint32_t word);

// Helper function to check for ODD parity
// Returns true if the parity is correct (odd number of 1s across the 32-bit word)
bool verify_parity(uint32_t word);

// -----------------------------------------------------------------------------
// 1. Altitude Simulation (Label 203 Octal - Barometric Altitude)
// Range: -1,000 to +131,072 ft. 
// Standard resolution (LSB) = 1.0 ft. Uses Bits 12-28 for data, Bit 29 for sign.
// -----------------------------------------------------------------------------
uint32_t encode_altitude(double altitude_ft, uint8_t sdi, uint8_t ssm);

DecodedArincWord decode_altitude(uint32_t word);

// -----------------------------------------------------------------------------
// 2. Bank Angle Simulation (Label 325 Octal - Roll/Bank Angle)
// Range: -180 to +180 deg.
// Scale: MSB (Bit 28) = 90 deg. 15 Bits of resolution (Bits 14-28). 
// LSB = 180 / 2^15 ≈ 0.0055 deg. Bit 29 is the sign bit (1 = Left/Minus, 0 = Right/Plus).
// -----------------------------------------------------------------------------
uint32_t encode_bank_angle(double angle_deg, uint8_t sdi, uint8_t ssm);

DecodedArincWord decode_bank_angle(uint32_t word);

// -----------------------------------------------------------------------------
// 3. Engine RPM Simulation (Label 341 Octal - Engine N1/N2 Percent RPM)
// Range: 0 to 110%. Unsigned. 
// Scale: Bits 11-28 (18 bits). MSB (Bit 28) = 64%. LSB ≈ 0.000488%.
// -----------------------------------------------------------------------------
uint32_t encode_engine_rpm(double rpm_percent, uint8_t sdi, uint8_t ssm);

DecodedArincWord decode_engine_rpm(uint32_t word);

#endif // ARINC_429_H