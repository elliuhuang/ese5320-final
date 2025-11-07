#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"

uint64_t hash_func(unsigned char * input, unsigned int pos) {
	uint64_t hash = 0;
	uint64_t prime_power = 3;
	for(int i = 0 ; i < WINDOW_SIZE; i++) {
		hash += input[pos + WINDOW_SIZE - 1 - i] * prime_power;
		prime_power *= 3;
	}
	return hash;
}

void cdc(unsigned char * block, int length, unsigned char * chunk_indices, int * num_chunks) {

}