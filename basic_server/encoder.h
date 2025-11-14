#ifndef _ENCODER_H_
#define _ENCODER_H_

// max number of elements we can get from ethernet
#define NUM_ELEMENTS 16384
#define HEADER 2
#define WINDOW_SIZE 16
#define BLOCKSIZE 8192
#define MAX_BUFFER_SIZE 8192
typedef struct{
    int16_t parent_key;
    int16_t val;
} Node;


#endif
