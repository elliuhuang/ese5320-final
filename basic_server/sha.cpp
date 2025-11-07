#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"

#include <openssl/sha.h>

void sha(unsigned char* chunk, int chunk_length, unsigned char* hash_num) {
    SHA256(chunk, chunk_length, hash_num);
}