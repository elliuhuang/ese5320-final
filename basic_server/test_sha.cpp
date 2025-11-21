#include <climits>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "encoder.h"
#include "sha_hw.cpp"

void print_hash(unsigned char* hash) {
    for (int i = 0; i < 48; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int main() {
    sha_hw_init();
    unsigned char hash[48];

    // Test 1: Empty string
    printf("Test 1: Empty string\n");
    unsigned char* test1 = (unsigned char*)"";
    sha_hw(test1, 0, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: 38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b\n\n");

    // Test 2: "abc"
    printf("Test 2: \"abc\"\n");
    unsigned char* test2 = (unsigned char*)"abc";
    sha_hw(test2, 3, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7\n\n");

    // Test 3: "hello world"
    printf("Test 3: \"hello world\"\n");
    unsigned char* test3 = (unsigned char*)"hello world";
    sha_hw(test3, 11, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: fdbd8e75a67f29f701a4e040385e2e23986303ea10239211af907fcbb83578b3e417cb71ce646efd0819dd8c088de1bd\n\n");

    // Test 4: Longer string
    printf("Test 4: \"The quick brown fox jumps over the lazy dog\"\n");
    unsigned char* test4 = (unsigned char*)"The quick brown fox jumps over the lazy dog";
    sha_hw(test4, 43, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: ca737f1014a48f4c0b6dd43cb177b0afd9e5169367544c494011e3317dbf9a509cb1e5dc1e85a941bbee3d7f2afbc9b1\n\n");

    // Test 5: String longer than 64 bytes
    printf("Test 5: Long string (> 64 bytes)\n");
    unsigned char* test5 = (unsigned char*)"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    sha_hw(test5, 112, hash);
    printf("Result:   ");
    print_hash(hash);
    printf("Expected: 09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039\n\n");

    printf("Test 6: name elliu\n");
    unsigned char* test6 = (unsigned char*)"elliu";
    sha_hw(test6, 5, hash);
    printf("Result:   " );
    print_hash(hash);
    printf("Expected: 1ffa0214a469bd6a8408a34c3909c4d29dd52f7ffd623cba2fe17b07efdbcb5aefe49696d96c1c03e21f23c1fadd302b\n\n");

    sha_hw_cleanup();
    return 0;
}
