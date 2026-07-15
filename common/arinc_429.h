#ifndef ARINC_429_H
#define ARINC_429_H

#include <stdint.h>

#define ARINC_ALTITUDE_LABEL 0x83
#define ARINC_RPM_LABEL 0xe4
#define ARINC_BANK_ANGLE_LABEL 0xd5
#define INTERVAL_US_429_HIGH_SPEED 3680000 // ~100kbps  ARINC 429 HIGH SPEED
#define INTERVAL_US_429_LOW_SPEED 29440000 // ~12.5kbps ARINC 429 LOW SPEED


/**
 * @brief ARINC 429 word representation.
 * Uses a union to allow access to individual bit-fields 
 * as well as the raw 32-bit payload.
 */
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

/**
 * @brief Generates a random ARINC 429 word structure.
 * @return arinc429_word_t The generated ARINC 429 word.
 */
arinc429_word_t generate_random_arinc(void);

/* --- Setters --- */
void arinc429_set_label(arinc429_word_t *word, uint32_t label);
void arinc429_set_sdi(arinc429_word_t *word, uint32_t sdi);
void arinc429_set_data(arinc429_word_t *word, uint32_t data);
void arinc429_set_ssm(arinc429_word_t *word, uint32_t ssm);
void arinc429_set_parity(arinc429_word_t *word, uint32_t parity);

/* --- Getters --- */
uint32_t arinc429_get_label(const arinc429_word_t *word);
uint32_t arinc429_get_sdi(const arinc429_word_t *word);
uint32_t arinc429_get_data(const arinc429_word_t *word);
uint32_t arinc429_get_ssm(const arinc429_word_t *word);
uint32_t arinc429_get_parity(const arinc429_word_t *word);

#endif // ARINC_429_H