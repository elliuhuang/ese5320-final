#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>

#include "encoder.h"

#ifndef SHA384_DIGEST_SZ
#define SHA384_DIGEST_SZ 48
#endif



// struct sockaddr_alg sa = {
//     .salg_family = AF_ALG,
//     .salg_type = "hash",
//     .salg_name = "sha3-384"
// };


struct sockaddr_alg sa;
int sockfd, fd;

void sha_hw(unsigned char* chunk, int chunk_length, unsigned char* hash_num) {
    unsigned char digest[SHA384_DIGEST_SZ];
    unsigned char remainder = chunk_length % 4;
    unsigned char padding = 4 - remainder;
    char *input = (char*) malloc(sizeof(unsigned char) * (chunk_length + padding));
    int input_size = chunk_length;

    // pad output to a multiple of 4 bytes
    if (remainder != 0) {
        memcpy(input, chunk, chunk_length);
        memset(input + chunk_length, 0, padding);
        input_size += padding;
    } else {
        memcpy(input, chunk, chunk_length);
    }

    /* Send Sha3 hash request with input data to driver */
    write(fd, input, input_size);
    /* Read the Sha3 digest output */
    read(fd, digest, SHA384_DIGEST_SZ);

    for(int i = 0; i < 48; i++) {
       hash_num[i] = digest[i];
    }
    // hash_num = digest;
    free(input);
}

void sha_hw_init() {

    sa.salg_family = AF_ALG;
    strncpy((char *)sa.salg_type ,"hash", sizeof(sa.salg_type));
    strncpy((char *)sa.salg_name ,"sha3-384", sizeof(sa.salg_name));

    sockfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
    /* Bind with SHA driver */
    (void) bind(sockfd, (struct sockaddr *)&sa, sizeof(sa));
    fd = accept(sockfd, NULL, 0);
}

void sha_hw_cleanup() {
    close(fd);
    close(sockfd);
}
