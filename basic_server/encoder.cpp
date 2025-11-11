#include "encoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "stopwatch.h"
#include "hash_table.h"

#include "cdc.cpp"
#include "sha.cpp"
#include "lzw.cpp"

#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)
#define MODULO uint64_t ((uint64_t)1 << 63)

int offset = 0;
unsigned char* file;

hash_map_t * global_hash_table;


void handle_input(int argc, char* argv[], int* blocksize, char** filename) {
	int x;
	extern char *optarg;

	while ((x = getopt(argc, argv, ":b:")) != -1) {
		switch (x) {
		case 'b':
			*blocksize = atoi(optarg);
			printf("blocksize is set to %d\n", *blocksize);
			break;
		case 'f':
			*filename = optarg;
			printf("filename is set to %s\n", *filename);
			break;
		case ':':
			printf("-%c without parameter\n", optopt);
			break;
		}
	}
}

void encode(unsigned char * block, int block_length) {
	// chunk_indices is the index of the end of each chunk
	// assume first chunk starts at index 0


	int * chunk_indices = (int *)malloc(sizeof(int) * BLOCKSIZE);
	int num_chunks = 0;
	cdc(block, block_length, chunk_indices, &num_chunks);

	int chunk_start = 0;

	for (int i = 0; i < num_chunks; i++) {
		int chunk_end = chunk_indices[i];
		int chunk_len = chunk_end - chunk_start;
		// int chunk_len = chunk_indices[i+1] - chunk_indices[i];
		unsigned char * hash_num = (unsigned char *)malloc(sizeof(unsigned char) * 32);
		// uint256_t hash_num = 0;
		sha(&block[chunk_start], chunk_len, hash_num);

		// this is dedup
		// hash_index is -1 if its a duplicate
		// otherwise it returns the index of the chunk that was encoded before
		int hash_index = search(global_hash_table, hash_num);

		
		// dedup(hash_num, &hash_index, hash_map_t global_hash_table); // hash_index is -1 if its a duplicate
		// no more dedup

		if (hash_index == -1) {
			unsigned char * compressed = (unsigned char*)malloc(sizeof(unsigned char) * BLOCKSIZE);
			int compressed_length;
			lzw(&block[chunk_start], chunk_len, compressed, &compressed_length);

			uint32_t compressed_header = compressed_length << 1;
			compressed_header &= ~(1);
			memcpy(&file[offset], &compressed_header, sizeof(compressed_header));
			offset += sizeof(compressed_header);

			memcpy(&file[offset], compressed, compressed_length);
			offset += compressed_length;

			insert(global_hash_table, hash_num);

			free(compressed);
		} else {
			uint32_t header = hash_index << 1;
			header |= 1;

			memcpy(&file[offset], &header, sizeof(header));
			offset += sizeof(header);
		}
		chunk_start = chunk_end;

		free(hash_num);
	}

	free(chunk_indices);
}

int main(int argc, char* argv[]) {
	stopwatch ethernet_timer;
	unsigned char* input[NUM_PACKETS];
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server;

	char* filename = strdup("output_cpu.bin");

	// default is 2k
	int blocksize = BLOCKSIZE;

	// set blocksize if decalred through command line
	handle_input(argc, argv, &blocksize, &filename);

	global_hash_table = (hash_map_t*)malloc(sizeof(hash_map_t));
	initializeHashMap(global_hash_table);

	file = (unsigned char*) malloc(sizeof(unsigned char) * 70000000);
	if (file == NULL) {
		printf("help\n");
	}

	for (int i = 0; i < NUM_PACKETS; i++) {
		input[i] = (unsigned char*) malloc(
				sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
		if (input[i] == NULL) {
			std::cout << "aborting " << std::endl;
			return 1;
		}
	}

	server.setup_server(blocksize);

	writer = pipe_depth;
	server.get_packet(input[writer]);
	count++;

	// get packet
	unsigned char* buffer = input[writer];


	// decode
	done = buffer[1] & DONE_BIT_L;
	length = buffer[0] | (buffer[1] << 8);
	length &= ~DONE_BIT_H;
	// printing takes time so be weary of transfer rate
	//printf("length: %d offset %d\n",length,offset);

	// we are just memcpy'ing here, but you should call your
	// top function here.
	encode(&buffer[HEADER], length);

	// memcpy(&file[offset], &buffer[HEADER], length);

	// offset += length;
	writer++;

	//last message
	while (!done) {
		// reset ring buffer
		if (writer == NUM_PACKETS) {
			writer = 0;
		}

		ethernet_timer.start();
		server.get_packet(input[writer]);
		ethernet_timer.stop();

		count++;

		// get packet
		unsigned char* buffer = input[writer];

		// decode
		done = buffer[1] & DONE_BIT_L;
		length = buffer[0] | (buffer[1] << 8);
		length &= ~DONE_BIT_H;
		//printf("length: %d offset %d\n",length,offset);
		// memcpy(&file[offset], &buffer[HEADER], length);
		encode(&buffer[HEADER], length);

		// offset += length;
		writer++;
	}

	// write file to root and you can use diff tool on board
	FILE *outfd = fopen(filename, "wb");
	int bytes_written = fwrite(&file[0], 1, offset, outfd);
	printf("write file with %d bytes\n", bytes_written);
	fclose(outfd);

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(file);
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float input_throughput = (bytes_written * 8 / 1000000.0) / ethernet_latency; // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;



	freeHashMap(global_hash_table);
	free(global_hash_table);

	return 0;
}
