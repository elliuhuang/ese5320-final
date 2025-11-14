#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"

#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13  // ceil(log2(8192)) = 13 bits
#define MAX_OUTPUT_SIZE (MAX_CHUNK_SIZE * CODE_LENGTH) / 8

void lzw_fpga(unsigned char *chunk, int chunk_len, unsigned char *compressed, int *compressed_length) {
    // interfaces
    #pragma HLS interface s_axilite port=return bundle=control
    #pragma HLS interface s_axilite port=chunk_len bundle=control
    #pragma HLS interface s_axilite port=compressed_length bundle=control
    #pragma HLS interface s_axilite port=chunk bundle=control
    #pragma HLS interface s_axilite port=compressed bundle=control
    #pragma HLS interface m_axi port=chunk offset=slave bundle=gmem0 depth=8192
    #pragma HLS interface m_axi port=compressed offset=slave bundle=gmem1 depth=13312

    // if the input is empty return nothing
    if (chunk_len == 0) {
        *compressed_length = 0;
        return;
    }

    // local arrays to be stored in BRAM
    unsigned char local_chunk[MAX_CHUNK_SIZE];
    unsigned char local_compressed[MAX_OUTPUT_SIZE];

    // get rid of struct
    int16_t dict_prefix[MAX_CHUNK_SIZE];
    unsigned char dict_byte[MAX_CHUNK_SIZE];
    #pragma HLS array_partition variable=dict_prefix cyclic factor=32
    #pragma HLS array_partition variable=dict_byte cyclic factor=32

    dict_setup_loop:
    for (int i = 0; i < 256; i++) {
        #pragma HLS pipeline
        dict_prefix[i] = -1;
        dict_byte[i] = (unsigned char) i;
    }
    int dict_size = 256;

    // Output buffer for codes
    uint16_t codes[MAX_CHUNK_SIZE];
    #pragma HLS bind_storage variable=codes type=ram_2p
    int code_count = 0;

    // copy data to BRAM buffer
    memcpy(local_chunk, chunk, chunk_len);

    // LZW encoding
    int16_t current = local_chunk[0];  // Start with first byte

    unsigned char next_byte;
    int16_t found;

    main_lzw_loop:
    for (int i = 1; i < chunk_len; i++) {
        #pragma HLS loop_tripcount min=1 max=8191
        dict_search_loop:
        for (int j = 0; j < MAX_CHUNK_SIZE; j++) {
            #pragma HLS pipeline
            #pragma HLS loop_tripcount min=1 max=8192

            // move this logic inside dict_search_loop so main_lzw_loop can be flattened
            if (j == 0) {
                next_byte = local_chunk[i];
                found = -1;
            }

            // combine into one if statement so it can be pipelined
            if (j < dict_size && dict_prefix[j] == current && dict_byte[j] == next_byte && found == -1) {
                found = j;
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
                dict_prefix[dict_size] = current;
                dict_byte[dict_size] = next_byte;
                dict_size++;
            }

            // Start new sequence with next_byte
            current = next_byte;
        }
    }

    // Output the last code
    codes[code_count++] = current;

    uint32_t bit_accumulator = 0;
    int bits_in_accumulator = 0;
    int byte_pos = 0;

    // Memset the output buffer (fast BRAM operation)
    memset(local_compressed, 0, MAX_OUTPUT_SIZE);

    code_packing_loop:
    for (int i = 0; i < code_count; i++) {
        #pragma HLS pipeline II=1
        #pragma HLS loop_tripcount min=1 max=8192

        // Add new code to accumulator
        bit_accumulator = (bit_accumulator << CODE_LENGTH) | codes[i];
        bits_in_accumulator += CODE_LENGTH;

        // Write bytes
        if (bits_in_accumulator >= 16) {
            // We have 16-20 bits, write 2 bytes
            
            // Write byte 1 (MSB)
            unsigned char output_byte_0 = (bit_accumulator >> (bits_in_accumulator - 8)) & 0xFF;
            local_compressed[byte_pos] = output_byte_0;
            
            // Write byte 2 (next)
            unsigned char output_byte_1 = (bit_accumulator >> (bits_in_accumulator - 16)) & 0xFF;
            local_compressed[byte_pos + 1] = output_byte_1;

            bits_in_accumulator -= 16;
            byte_pos += 2;

        } else if (bits_in_accumulator >= 8) {
            // We have 8-15 bits, write 1 byte
            
            // Write byte 1 (MSB)
            unsigned char output_byte = (bit_accumulator >> (bits_in_accumulator - 8)) & 0xFF;
            local_compressed[byte_pos] = output_byte;
            
            bits_in_accumulator -= 8;
            byte_pos += 1;
        }
        // else (bits_in_accumulator < 8) -> do nothing, hold bits
    }

    // Flush any remaining bits
    if (bits_in_accumulator > 0) {
        // Shift the remaining bits to the MSB side of the byte
        unsigned char last_byte = (bit_accumulator << (8 - bits_in_accumulator)) & 0xFF;
        local_compressed[byte_pos++] = last_byte;
    }
    *compressed_length = byte_pos;

    // copy back out of the BRAM buffers
    memcpy(compressed, local_compressed, byte_pos);
}
