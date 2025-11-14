#include "stopwatch.h"
#include "encoder.h"
#include "hash_table.h"
#include "server.h"
#include "Utilities.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>
// #include <CL/cl2.hpp>


void lzw_fpga(cl_uchar *chunk, int chunk_len, cl_uchar *compressed, int *compressed_length);
void sha(unsigned char* chunk, int chunk_length, unsigned char* hash_num);
void cdc(unsigned char *block, int length, int *chunk_indices, int *num_chunks);

