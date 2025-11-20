#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"

#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13  // ceil(log2(8192)) = 13 bits

// Dictionary entry structure
typedef struct {
    int prefix;      // -1 for single bytes, otherwise index of prefix
    unsigned char byte;
} DictEntry;

void lzw_hw(unsigned char *chunk, int chunk_len, unsigned char *compressed, int *compressed_length) {
    #pragma HLS INTERFACE m_axi port=chunk bundle=gmem0 depth=8192
    #pragma HLS INTERFACE m_axi port=compressed bundle=gmem1 depth=16384
    #pragma HLS INTERFACE s_axilite port=chunk_len
    #pragma HLS INTERFACE s_axilite port=compressed_length
    #pragma HLS INTERFACE s_axilite port=return

    // Use static arrays for BRAM storage
    static DictEntry dictionary[MAX_CHUNK_SIZE];
    #pragma HLS BIND_STORAGE variable=dictionary type=ram_2p impl=bram
    #pragma HLS ARRAY_PARTITION variable=dictionary cyclic factor=4 dim=1

    static uint16_t codes[MAX_CHUNK_SIZE];
    #pragma HLS BIND_STORAGE variable=codes type=ram_2p impl=bram
    #pragma HLS ARRAY_PARTITION variable=codes cyclic factor=2 dim=1

    if (chunk_len == 0) {
        *compressed_length = 0;
        return;
    }

    // Initialize dictionary with 256 single-byte entries
    INIT_DICT: for (int i = 0; i < 256; i++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS UNROLL factor=4
        dictionary[i].prefix = -1;
        dictionary[i].byte = (unsigned char)i;
    }
    int dict_size = 256;
    int code_count = 0;

    // LZW encoding
    int current = chunk[0];  // Start with first byte

    ENCODE_LOOP: for (int i = 1; i < chunk_len; i++) {
        #pragma HLS LOOP_TRIPCOUNT min=2048 max=8192 avg=4096
        #pragma HLS PIPELINE off

        unsigned char next_byte = chunk[i];

        // Search for current + next_byte in dictionary
        int found = -1;

        SEARCH_DICT: for (int j = 0; j < dict_size; j++) {
            #pragma HLS LOOP_TRIPCOUNT min=256 max=8192 avg=2048
            #pragma HLS PIPELINE II=1

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

    // Clear output buffer
    CLEAR_OUTPUT: for (int i = 0; i < MAX_CHUNK_SIZE; i++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS UNROLL factor=8
        compressed[i] = 0;
    }

    // Pack codes into bytes (MSB-first, as specified)
    int bit_pos = 0;  // Current bit position in output
    int byte_pos = 0; // Current byte position in output

    PACK_CODES: for (int i = 0; i < code_count; i++) {
        #pragma HLS LOOP_TRIPCOUNT min=512 max=8192 avg=2048
        #pragma HLS PIPELINE II=13

        uint16_t code = codes[i];

        // Write CODE_LENGTH bits, MSB first
        PACK_BITS: for (int bit = CODE_LENGTH - 1; bit >= 0; bit--) {
            #pragma HLS UNROLL

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
}
