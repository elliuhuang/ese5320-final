#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"

#define MIN_CHUNK_SIZE 2048
#define AVG_CHUNK_SIZE 4096
#define MAX_CHUNK_SIZE 8192
#define START (WINDOW_SIZE - 7) * 8

#define POLYNOMIAL 0x3DA3358B4DC173  // Irreducible polynomial


// Mask for determining chunk boundaries average 4KB size
#define CHUNK_MASK ((1ULL << 12) - 1)  // 0xFFF - gives ~4KB average chunks
__uint128_t poly = POLYNOMIAL;


//32 byte chunks from the block and divide by the irreducible polynomial
bool hash(__uint128_t D) {

    __uint128_t poly = POLYNOMIAL;
    poly = poly << START;
    for(int i = START; i >= 0; i--) {
        D = D ^ poly;
        if(i != 0) {
            D = D << 1;
        } 

    }

    if(((D >> START) & CHUNK_MASK) == CHUNK_MASK ) {
        return true;
    
    } else {
        return false;
    }


}
void cdc(unsigned char *block, int length, int *chunk_indices, int *num_chunks) {
    
    int count = 0;
    int index = 0;
    int prev_index = 0;
    __uint128_t D = 0;
    unsigned char window[WINDOW_SIZE];
    for(int i = 0; i < WINDOW_SIZE; i++) {
        D = (D << 8) | block[i];
    }
    while(index < length - WINDOW_SIZE) {

        if((index - prev_index > MAX_CHUNK_SIZE) || ((index - prev_index > MIN_CHUNK_SIZE) && hash(D))) {

            chunk_indices[count] = index;
            count++;
            prev_index = index;
        }

        index++;
        D = D << 8;
        D = D | block[index +WINDOW_SIZE - 1];       


    }
    *num_chunks = count;

}




































//Chats code:
        // // Precomputed table for the polynomial
        // static uint64_t poly_table[256];
        // static int table_initialized = 0;

        // // Initialize the polynomial lookup table for faster computation
        // static void init_poly_table() {
        //     if (table_initialized) return;

        //     for (int i = 0; i < 256; i++) {
        //         uint64_t hash = i;
        //         for (int j = 0; j < 8; j++) {
        //             if (hash & 1) {
        //                 hash = (hash >> 1) ^ POLYNOMIAL;
        //             } else {
        //                 hash >>= 1;
        //             }
        //         }
        //         poly_table[i] = hash;
        //     }
        //     table_initialized = 1;
        // }

        // // Compute hash of a single byte using the lookup table
        // static inline uint64_t hash_byte(uint64_t hash, unsigned char byte) {
        //     return (hash >> 8) ^ poly_table[(hash ^ byte) & 0xFF];
        // }

        // // Rolling hash: add a new byte and remove the oldest byte
        // static inline uint64_t roll_hash(uint64_t hash, unsigned char new_byte,
        //                                   unsigned char old_byte, uint64_t old_byte_hash) {
        //     hash ^= old_byte_hash;
        //     hash = hash_byte(hash, new_byte);
        //     return hash;
        // }

        // // Precompute the hash contribution of a byte at the beginning of the window
        // static uint64_t compute_old_byte_hash(unsigned char byte) {
        //     uint64_t hash = poly_table[byte];
        //     // Apply the polynomial WINDOW_SIZE-1 times
        //     for (int i = 1; i < WINDOW_SIZE; i++) {
        //         hash = hash_byte(hash, 0);
        //     }
        //     return hash;
        // }

        // void cdc(unsigned char *block, int length, int *chunk_indices, int *num_chunks) {
        //     if (length == 0) {
        //         *num_chunks = 0;
        //         return;
        //     }

        //     init_poly_table();

        //     int chunk_count = 0;
        //     int current_pos = 0;
        //     int last_chunk_end = 0;

        //     // Initial hash computation for the first window
        //     uint64_t hash = 0;
        //     unsigned char window[WINDOW_SIZE];
        //     int window_idx = 0;

        //     // Fill initial window or use available data
        //     int initial_window_size = (length < WINDOW_SIZE) ? length : WINDOW_SIZE;
        //     for (int i = 0; i < initial_window_size; i++) {
        //         hash = hash_byte(hash, block[i]);
        //         window[i] = block[i];
        //     }
        //     window_idx = initial_window_size % WINDOW_SIZE;
        //     current_pos = initial_window_size;

        //     // Scan through the block looking for chunk boundaries
        //     while (current_pos < length) {
        //         int chunk_size = current_pos - last_chunk_end;

        //         // Check if we've reached minimum chunk size before looking for boundaries
        //         if (chunk_size >= MIN_CHUNK_SIZE) {
        //             // Check if this is a chunk boundary (hash matches the mask)
        //             if ((hash & CHUNK_MASK) == 0) {
        //                 // Found a boundary!
        //                 chunk_indices[chunk_count++] = current_pos;
        //                 last_chunk_end = current_pos;
        //             }
        //             // Force a cut if we've reached max chunk size
        //             else if (chunk_size >= MAX_CHUNK_SIZE) {
        //                 chunk_indices[chunk_count++] = current_pos;
        //                 last_chunk_end = current_pos;
        //             }
        //         }

        //         // Move to next byte
        //         if (current_pos < length) {
        //             unsigned char new_byte = block[current_pos];
        //             unsigned char old_byte = window[window_idx];

        //             // Compute the hash contribution of the old byte
        //             uint64_t old_byte_hash = compute_old_byte_hash(old_byte);

        //             // Roll the hash
        //             hash = roll_hash(hash, new_byte, old_byte, old_byte_hash);

        //             // Update the window
        //             window[window_idx] = new_byte;
        //             window_idx = (window_idx + 1) % WINDOW_SIZE;
        //             current_pos++;
        //         }
        //     }

        //     // Add the final chunk if there's remaining data
        //     if (last_chunk_end < length) {
        //         chunk_indices[chunk_count++] = length;
        //     }

        //     *num_chunks = chunk_count;
        // }
