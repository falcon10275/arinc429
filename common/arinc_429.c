#include "arinc_429.h"
#include <stdlib.h>

/* --- Setters --- */
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

/* --- Getters --- */
uint32_t arinc429_get_label(const arinc429_word_t *word) {
    return word->fields.label;
}

uint32_t arinc429_get_sdi(const arinc429_word_t *word) {
    return word->fields.sdi;
}

uint32_t arinc429_get_data(const arinc429_word_t *word) {
    return word->fields.data;
}

uint32_t arinc429_get_ssm(const arinc429_word_t *word) {
    return word->fields.ssm;
}

uint32_t arinc429_get_parity(const arinc429_word_t *word) {
    return word->fields.parity;
}

/* --- Utilities --- */
arinc429_word_t generate_random_arinc(void) {
    arinc429_word_t word;
    word.raw = 0; // Initialize to 0

    // Utilize our new setters to populate the random data
    arinc429_set_label(&word, rand());
    arinc429_set_sdi(&word, rand());
    arinc429_set_data(&word, rand());
    arinc429_set_ssm(&word, rand());
    arinc429_set_parity(&word, rand());

    return word;
}