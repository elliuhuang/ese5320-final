#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"


#define MAX_CHUNK_SIZE 8192
#define CODE_LENGTH 13  // ceil(log2(8192)) = 13 bits



//compressed gives the leaf of each tree that the chunk has which is decoded using the tree array.
void lzw(unsigned char *chunk, int chunk_len, int16_t *compressed, int *compressed_length, Node *tree) {

    if(chunk_len == 0) {
        throw "Zero sized chunk";
    }
    int16_t dictionary_index = 0;
    int chunk_index = 0;
    int count = 0;
    int16_t dictionary[4096][256];
    //Node tree[4096];
    for(dictionary_index = 0; dictionary_index < 256; dictionary_index++) {
        tree[dictionary_index].parent_key = -1;
        tree[dictionary_index].val = dictionary_index;
    }

    for (int i = 0; i < 4096; ++i) {
        for (int j = 0; j < 256; ++j) {
            dictionary[i][j] = -1;
        }
    }

    //int16_t compressed[chunk_len];

    int16_t key = (int16_t)chunk[chunk_index];
    chunk_index++;


    while(chunk_index < chunk_len) {
        int16_t chunk_char = chunk[chunk_index];
        while(dictionary[key][chunk_char] != -1) {
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

        key = (int16_t)chunk[chunk_index];
        chunk_index++;
        count++;
    }

    *compressed_length = count;
 
}

int main(){
    unsigned char chunk[] = "I AM SAM SAM I AM";
    int16_t comp[17];
    int compressed_length = 0;
    Node tree[4096];

    lzw(chunk, 17, comp, &compressed_length, tree);

    for(int i = 0; i < compressed_length; i++) {
    //     //printf("index %d: %d\n", i, tree[comp[i]].val);
           //printf("%c, " ,tree[comp[i]].val);
           //printf("%d, " ,tree[comp[i]].parent_key);

           //printf("\n");
         int temp = i;
        while(tree[comp[temp]].parent_key != -1) {
            // printf("%c, ",tree[comp[temp]].val);
             temp = tree[comp[temp]].parent_key;
            printf("%c, ",tree[comp[temp]].val);

             //printf("in while\n");
       }
       printf("%c, ",tree[comp[temp]].val);
       
    }
    printf("\n");
 
    
    return 0;
}