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
#define SHA384_DIGEST_SZ 384
#endif

struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "hash",
    .salg_name = "sha3-384"
};

int sockfd, fd;

void sha_hw(unsigned char* chunk, int chunk_length, unsigned char* hash_num) {
    unsigned char digest[SHA384_DIGEST_SZ];
    char *input = (char* ) chunk; /* Input Data should be multiple of 4-bytes */

    // pad output to a multiple of 4 bytes
    unsigned char remainder = chunk_length % 4;
    if (remainder != 0) {
        unsigned char padding = 4 - remainder;

        input = (char*) malloc(sizeof(unsigned char) * (chunk_length + padding));

        memcpy(input, chunk, chunk_length);
        memset(input + chunk_length, 0, padding);
    }

    /* Send Sha3 hash request with input data to driver */
    write(fd, input, strlen(input));
    /* Read the Sha3 digest output */
    read(fd, digest, SHA384_DIGEST_SZ);

    hash_num = digest;
}

void sha_hw_init() {
    sockfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
    /* Bind with SHA driver */
    (void) bind(sockfd, (struct sockaddr *)&sa, sizeof(sa));
    fd = accept(sockfd, NULL, 0);
}

void sha_hw_cleanup() {
    close(fd);
    close(sockfd);
}
