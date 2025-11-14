

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "encoder.h"


#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13  // ceil(log2(8192)) = 13 bits
// Dictionary entry structure
typedef struct {
    int prefix;      // -1 for single bytes, otherwise index of prefix
    unsigned char byte;
} DictEntry;
void lzw_fpga(unsigned char *chunk, int chunk_len, unsigned char *compressed, int *compressed_length) {
    if (chunk_len == 0) {
        *compressed_length = 0;
        return;
    }
    // Initialize dictionary with 256 single-byte entries
    DictEntry dictionary[sizeof(DictEntry) * MAX_CHUNK_SIZE];
    for (int i = 0; i < 256; i++) {
        dictionary[i].prefix = -1;
        dictionary[i].byte = (unsigned char)i;
    }
    int dict_size = 256;
    // Output buffer for codes
    uint16_t codes[sizeof(uint16_t) * MAX_CHUNK_SIZE];
    int code_count = 0;
    // LZW encoding
    int current = chunk[0];  // Start with first byte
    for (int i = 1; i < chunk_len; i++) {
        unsigned char next_byte = chunk[i];
        // Search for current + next_byte in dictionary
        int found = -1;
        for (int j = 0; j < dict_size; j++) {
            if (dictionary[j].prefix == current && dictionary[j].byte == next_byte) {
                found = j;
                break;
            }
        }
        if (found != -1) {
            // Found in dictionary, extend current
            current = found;
        } else {
            // Not found, output current code
            codes[code_count++] = current;
            // Add new entry to dictionary
            if (dict_size < MAX_CHUNK_SIZE) {
                dictionary[dict_size].prefix = current;
                dictionary[dict_size].byte = next_byte;
                dict_size++;
            }
            // Start new sequence with next_byte
            current = next_byte;
        }
    }
    // Output the last code
    codes[code_count++] = current;
    // Pack codes into bytes (MSB-first, as specified)
    int bit_pos = 0;  // Current bit position in output
    int byte_pos = 0; // Current byte position in output
    memset(compressed, 0, MAX_CHUNK_SIZE);
    for (int i = 0; i < code_count; i++) {
        uint16_t code = codes[i];
        // Write CODE_LENGTH bits, MSB first
        for (int bit = CODE_LENGTH - 1; bit >= 0; bit--) {
            int bit_value = (code >> bit) & 1;
            if (bit_value) {
                compressed[byte_pos] |= (1 << (7 - bit_pos));
            }
            bit_pos++;
            if (bit_pos == 8) {
                bit_pos = 0;
                byte_pos++;
            }
        }
    }
    // If we're not on a byte boundary, move to next byte (padding already zeros)
    if (bit_pos != 0) {
        byte_pos++;
    }
    *compressed_length = byte_pos;
    // free(dictionary);
    // free(codes);
}
