#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"
#include "sha.cpp"

void print_hash(unsigned char* hash) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int main() {
    unsigned char hash[32];

    // Test 1: Empty string
    printf("Test 1: Empty string\n");
    unsigned char* test1 = (unsigned char*)"";
    sha(test1, 0, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n\n");

    // Test 2: "abc"
    printf("Test 2: \"abc\"\n");
    unsigned char* test2 = (unsigned char*)"abc";
    sha(test2, 3, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n\n");

    // Test 3: "hello world"
    printf("Test 3: \"hello world\"\n");
    unsigned char* test3 = (unsigned char*)"hello world";
    sha(test3, 11, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9\n\n");

    // Test 4: Longer string
    printf("Test 4: \"The quick brown fox jumps over the lazy dog\"\n");
    unsigned char* test4 = (unsigned char*)"The quick brown fox jumps over the lazy dog";
    sha(test4, 43, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592\n\n");

    // Test 5: String longer than 64 bytes
    printf("Test 5: Long string (> 64 bytes)\n");
    unsigned char* test5 = (unsigned char*)"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    sha(test5, 112, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1\n\n");

    printf("Test 6: name elliu\n");
    unsigned char* test6 = (unsigned char*)"elliu";
    sha(test6, 5, hash);
    printf("Result:   " );
    print_hash(hash);
    printf("Expected: 0b27b67208d3fbfbe636925b26203aec982399f8aa0b1b1a85045fbfc5c8b6fc\n\n");


    return 0;
}
