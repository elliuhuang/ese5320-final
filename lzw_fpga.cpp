#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"


#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13  // ceil(log2(8192)) = 13 bits


typedef struct{
    uint16_t parent_key;
    uint16_t val;
} Node;

//compressed gives the leaf of each tree that the chunk has which is decoded using the tree array.
void lzw(unsigned char *chunk, int chunk_len, uint16_t *compressed, int *compressed_length, Node *tree) {

    if(chunk_len == 0) {
        throw "Zero sized chunk";
    }
    uint16_t dictionary_index = 0;
    int chunk_index = 0;
    int count = 0;
    uint16_t dictionary[4096][256];
    //Node tree[4096];
    for(dictionary_index; dictionary_index < 256; dictionary_index++) {
        tree[dictionary_index].parent_key = NULL;
        tree[dictionary_index].val = dictionary_index;
    }

    //uint16_t compressed[chunk_len];

    uint16_t key = (uint16_t)chunk[chunk_index]; 
    chunk_index++;


    while(chunk_index < chunk_len) {
        uint16_t chunk_char = chunk[chunk_index];
        while(dictionary[key][chunk_char] != NULL) {
            key = dictionary[key][chunk_char];

            chunk_index++;
            chunk_char = chunk[chunk_index];
        }
        //Send key
        compressed[count] = key;
        //Store everything here

        tree[dictionary_index].parent_key = key;
        tree[dictionary_index].val = chunk_char;

        dictionary[key][chunk_char] = dictionary_index++;

        key = (u_int16_t)chunk[chunk_index];
        chunk_index++;
        count++;
    }

    *compressed_length = count;
    


} 