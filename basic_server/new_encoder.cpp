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

#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)
#define WINDOW_SIZE 17
#define MODULO uint64_t ((uint64_t)1 << 63)

int offset = 0;
unsigned char* file;
uint64_t hash_table[NUM_ELEMENTS];
int chunkCount;

void handle_input(int argc, char* argv[], int* blocksize) {
	int x;
	extern char *optarg;

	while ((x = getopt(argc, argv, ":b:")) != -1) {
		switch (x) {
		case 'b':
			*blocksize = atoi(optarg);
			printf("blocksize is set to %d optarg\n", *blocksize);
			break;
		case ':':
			printf("-%c without parameter\n", optopt);
			break;
		}
	}
}

void cdc(unsigned char * IN, unsigned char* OUT, int length);
void sha_hash(unsigned char *IN, int chunk_indx_start, int chunk_indx_end, unsigned char *chunk_ptr, int* chunk_size);
// void sha_hash(unsigned char *IN, unsigned char *OUT, int length);
// void lzw(unsigned char* IN, unsigned char* OUT, int length);
void lzw(unsigned char *IN, unsigned char *OUT, int chunk_indx_start, int chunk_indx_end, uint64_t hash_num);


int main(int argc, char* argv[]) {
	stopwatch ethernet_timer;
	stopwatch cdc_timer;
	stopwatch hash_timer;
	stopwatch lzw_timer;
	unsigned char* input[NUM_PACKETS];
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server;

	// default is 2k
	int blocksize = BLOCKSIZE;

	// set blocksize if decalred through command line
	handle_input(argc, argv, &blocksize);

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
	unsigned char* Temp1= (unsigned char *) malloc(sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
	unsigned char* Temp2 = (unsigned char *) malloc(sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
	unsigned char* outBuffer = (unsigned char *) malloc(sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));

	// decode
	done = buffer[1] & DONE_BIT_L;
	length = buffer[0] | (buffer[1] << 8);
	length &= ~DONE_BIT_H;
	// printing takes time so be weary of transfer rate
	//printf("length: %d offset %d\n",length,offset);

	// we are just memcpy'ing here, but you should call your
	// top function here.
	cdc(buffer, outBuffer, length);
	// sha_hash(Temp1, Temp2, length);
	// lzw(Temp1, outBuffer, length);
	memcpy(&file[offset], &outBuffer[HEADER], length);
	// memcpy(&file[offset], &buffer[HEADER], length);

	offset += length;
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

		int chunkCount = 999;
		cdc_timer.start();
		cdc(buffer,outBuffer, length);
		cdc_timer.stop();

		// std::cout << "Number of chunks: " << chunkCount << std::endl;

		// sha_hash(Temp1, Temp2, length);


		lzw_timer.start();
		// lzw(Temp1, outBuffer, length);
		lzw_timer.stop();




		// decode
		done = outBuffer[1] & DONE_BIT_L;
		length = outBuffer[0] | (outBuffer[1] << 8);
		length &= ~DONE_BIT_H;
		//printf("length: %d offset %d\n",length,offset);
		memcpy(&file[offset], &outBuffer[HEADER], length);

		offset += length;
		writer++;
	}

	// write file to root and you can use diff tool on board
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int bytes_written = fwrite(&file[0], 1, offset, outfd);
	printf("write file with %d\n", bytes_written);
	fclose(outfd);

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(file);
	free(Temp1);
	free(Temp2);
	free(outBuffer);

	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float cdc_latency = cdc_timer.latency() / 1000.0;
	//float hash_latency = hash_timer.latency() / 1000.0;
	float lzw_latency = lzw_timer.latency() / 1000.0;

	float input_throughput = (bytes_written * 8 / 1000000.0) / (ethernet_latency + cdc_latency + lzw_latency); // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;

	return 0;
}

uint64_t hash_func(unsigned char * input, unsigned int pos) {
	uint64_t hash = 0;
	uint64_t prime_power = 3;
	for(int i = 0 ; i < WINDOW_SIZE; i++) {
		hash += input[pos + WINDOW_SIZE - 1 - i] * prime_power;
		prime_power *= 3;
	}
	return hash;
}
void cdc(unsigned char *IN, unsigned char *OUT, int length) {

	unsigned char * chunk_ptr = (unsigned char *) malloc(sizeof(unsigned char) * (length + HEADER));
	int prev_chunk_index = 0;
	int chunk_index = 0;

// 	for (int i = 0; i < length + HEADER - WINDOW_SIZE; i+=WINDOW_SIZE) {
// 		for (j = i; j < WINDOW_SIZE; j++) {
// 			if (chunk_index % 256 == 0) {
// 				sha_hash(IN, prev_chunk_index, chunk_index, chunk_ptr, &out_size);
// 				prev_chunk_index = chunk_index + 1;
// 			}
// 			chunk_index++;
// 		}
//
// 	}
	bool first_chunk = 1;
	chunkCount = 0;
	//bool chunk_started = 1;


	int out_size = 0;
	int prev_out_index = 0;
	int out_index = 0;
	for(int i = WINDOW_SIZE; i < length + HEADER - WINDOW_SIZE; i++) {
		// if((chunk_index % (256)) == 0) {
		if((hash_func(IN, i) % (256)) == 0) {
			//sha_hash();
			//chunk_started = !chunk_started;

			sha_hash(IN, prev_chunk_index, chunk_index, chunk_ptr, &out_size);
			chunkCount++;

			out_index = prev_out_index + out_size;

			for(int i = prev_out_index; i < out_index; i++) {

				OUT[i] = chunk_ptr[i - prev_out_index];
			}
			prev_out_index = out_index;
			prev_chunk_index = chunk_index;
		}
		chunk_index++;
	}
	sha_hash(IN, prev_chunk_index, length + HEADER, chunk_ptr, &out_size);
	chunkCount++;

	out_index = prev_out_index + out_size;

	for(int i = prev_out_index; i < out_index; i++) {

		OUT[i] = chunk_ptr[i - prev_out_index];
	}
	// for (int i = 0; i < length+HEADER; i++) {
	// 	OUT[i] = IN[i];
	// }
	std::cout << "cdc chunk count: "<< chunkCount << std::endl;
	free(chunk_ptr);
	// return chunkCount;
}
void sha_hash(unsigned char *IN, int chunk_indx_start, int chunk_indx_end, unsigned char *chunk_ptr, int* chunk_size) {
	*chunk_size = chunk_indx_end - chunk_indx_start;
	// HASH here
	int x = 0;
	uint64_t sum = 0;
	uint64_t temp = 0;

	for(x = chunk_indx_start; x < chunk_indx_end - 8; x+=8) {

		for( int j = 0; j < 8; j ++) {
			temp += (IN[x + j]) << (8 * j);
		}
		sum += temp % (MODULO);
		temp = 0;
	}
	for(int i = x; x < chunk_indx_end; x ++) {
		temp += (IN[i]) << (8 * (i - x));
	}
	sum += temp % (MODULO);
//
	// for(int i = chunk_indx_start; i < chunk_indx_end; i++) {
	// 	chunk_ptr[i - chunk_indx_start] = IN[i];
	// }
	lzw(IN, chunk_ptr, chunk_indx_start, chunk_indx_end, sum);
	// for (int i = 0; i < length+HEADER; i++) {
	// 	OUT[i] = IN[i];
	// }
	// std::cout << "doing sha hash "<< std::endl;

}

void lzw(unsigned char *IN, unsigned char *OUT, int chunk_indx_start, int chunk_indx_end, uint64_t hash_num) {
	bool found = false;
	for (int i = 0; i < chunkCount; i++) {
		if (hash_table[i] == hash_num) {
			found = true;
		}
	}
	if (!found) {
		hash_table[chunkCount] = hash_num;
	}
	for(int i = chunk_indx_start; i < chunk_indx_end; i++) {
		OUT[i - chunk_indx_start] = IN[i];
	}
	// for(int i = 0; i < length+HEADER; i++) {
	// 	OUT[i] = IN[i];
	// }

	// std::cout << "doing lzw "<< std::endl;

}





