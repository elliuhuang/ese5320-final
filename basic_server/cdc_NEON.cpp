// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <iostream>
// #include "encoder.h"
// #include <arm_neon.h>

// #define MIN_CHUNK_SIZE 2048
// #define AVG_CHUNK_SIZE 4096
// #define MAX_CHUNK_SIZE 8192
// #define START (WINDOW_SIZE - 7) * 8

// #define POLYNOMIAL 0x3DA3358B4DC173  // Irreducible polynomial


// // Mask for determining chunk boundaries average 4KB size
// #define CHUNK_MASK ((1ULL << 12) - 1)  // 0xFFF - gives ~4KB average chunks

// static uint8x16_t  poly = (POLYNOMIAL << START);


// //32 byte chunks from the block and divide by the irreducible polynomial
// bool hash(uint8x16_t D) {

    
//     for(int i = START; i >= 0; i--) {
//         D = veorq_u8(D,poly);
//         if(i != 0) {
//             D = D << 1;
//         } 

//     }

//     if(((D >> START) & CHUNK_MASK) == CHUNK_MASK ) {
//         return true;
    
//     } else {
//         return false;
//     }


// }
// void cdc(unsigned char *block, int length, int *chunk_indices, int *num_chunks) {
    
//     int count = 0;
//     int index = 0;
//     int prev_index = 0;
//     uint8x16_t D = 0;
//     D = vld1q_u8(block);

//     while(index < length - WINDOW_SIZE) {

//         if((index - prev_index > MAX_CHUNK_SIZE) || ((index - prev_index > MIN_CHUNK_SIZE) && hash(D))) {

//             chunk_indices[count] = index;
//             count++;
//             prev_index = index;
//         }
//         //Improve to a circular load but for now just shift
//         index++;
//         vshlq_u8(D, D); //If I can't use a temp variable
//         vld1q_lane_u8(block + index, D, WINDOW_SIZE - 1);

//     }
//     *num_chunks = count;

// }



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"
//#include <arm_neon.h>

#define MIN_CHUNK_SIZE 2048
#define AVG_CHUNK_SIZE 4096
#define MAX_CHUNK_SIZE 8192
#define POLY_SIZE 10

//#define START (WINDOW_SIZE - POLY_SIZE) * 8

//#define POLYNOMIAL 0x3DA3358B4DC173  // Irreducible polynomial
#define WINDOW_SIZE 8
#define POLYNOMIAL 0x3DA3358B4DC173
#define START (WINDOW_SIZE * 8 - 40)

// Mask for determining chunk boundaries average 4KB size
#define CHUNK_MASK ((1ULL << 12) - 1)  // 0xFFF - gives ~4KB average chunks
uint64_t p = POLYNOMIAL;
uint64_t poly = p << START;
uint64_t c_m = CHUNK_MASK;
uint64_t chunk_mask = c_m << START;


//32 byte chunks from the block and divide by the irreducible polynomial
bool hash(uint64_t D) {

  for(int i = START; i > 0; i--) {
        D = D ^ poly;
        D = D << 1;
    }
    D = D ^ poly;

    if((D & chunk_mask) == chunk_mask) {

        return true;
    
    } else {
        return false;
    }


}
void cdc(unsigned char *block, int length, int *chunk_indices, int *num_chunks) {
    
    int count = 0;
    int index = 0;
    int prev_index = 0;
    uint64_t D = 0;
    unsigned char window[WINDOW_SIZE];
    //Keep like this because we will be streaming this in?? If not we want to unroll
    D = block[0] | (block[1]  << 8) | (block[2]  << 16) | (block[3]  << 24) | (block[4]  << 32)| (block[5]  << 40)
    | (block[6]  << 48)| (block[7]  << 56)| (block[8]  << 64)| (block[9]  << 72)| (block[10]  << 80)
    | (block[11]  << 88)| (block[12]  << 96)| (block[13]  << 104)| (block[14]  << 112)| (block[15]  << 120);

    // for(int i = 0; i < WINDOW_SIZE; i++) {
    //     D = (D << 8) | block[i];
    // }
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

