 #define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "host.h"


#define NUM_PACKETS 8192
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)
#define MODULO uint64_t ((uint64_t)1 << 63)

int offset = 0;
unsigned char* file;

unsigned char hold[sizeof(unsigned char) * (NUM_ELEMENTS + HEADER) * 2];
int hold_index = 0;

hash_map_t * global_hash_table;

int lzw_chunk_count = 0;
int total_input_bytes = 0;

stopwatch cdc_timer, sha_timer, dedup_timer, lzw_timer;


void handle_input(int argc, char* argv[], int* blocksize, char** filename);
void encode(unsigned char * block, int block_length, int done, cl::CommandQueue q, cl::Kernel krnl_lzw, cl::Context context,
                cl_int* err);

int main(int argc, char *argv[])
{
    //Server communicatoin setup
        stopwatch ethernet_timer, encode_timer;
        unsigned char* input[NUM_PACKETS];
        int writer = 0;
        int done = 0;
        int length = 0;
        int count = 0;
        lzw_chunk_count = 0;
        ESE532_Server server;

        char* filename = strdup("output_cpu.bin");

        // default is 2k
        int blocksize = BLOCKSIZE;

        // set blocksize if decalred through command line
        handle_input(argc, argv, &blocksize, &filename);

        global_hash_table = (hash_map_t*)malloc(sizeof(hash_map_t));
        initializeHashMap(global_hash_table);

        file = (unsigned char*) malloc(sizeof(unsigned char) * 700000000);
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

        total_input_bytes += length;

    //ENVIRONMENT INIT
    cl_int err;
    std::string binaryFile = /*argv[1]*/ "lzw_hw.xclbin";
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char *fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE , &err);
    cl::Kernel krnl_lzw(program, "lzw_hw", &err);

    encode_timer.start();
    encode(&buffer[HEADER], length, done, q, krnl_lzw, context, &err);
    encode_timer.stop();

    writer++;

    	while (!done) {
		// reset ring buffer
		if (writer == NUM_PACKETS) {
			writer = 0;
		}

		ethernet_timer.start();
		server.get_packet(input[writer]);
		ethernet_timer.stop();

		count++;

		if (count % 100 == 0) {
			printf("Received %d packets...\n", count);
		}

		// get packet
		unsigned char* buffer = input[writer];

		// decode
		done = buffer[1] & DONE_BIT_L;
		length = buffer[0] | (buffer[1] << 8);
		length &= ~DONE_BIT_H;

        total_input_bytes += length;
		//printf("length: %d offset %d\n",length,offset);
		// memcpy(&file[offset], &buffer[HEADER], length);
		// encode(&buffer[HEADER], length, done);
        encode_timer.start();
        encode(&buffer[HEADER], length, done, q, krnl_lzw, context, &err);
        encode_timer.stop();

		// offset += length;
		writer++;
	}
    	printf("\n=== RECEPTION COMPLETE ===\n");
	printf("Total packets received: %d\n", count);
	printf("==========================\n\n");

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
	float encode_latency = encode_timer.latency() / 1000.0;
	float cdc_latency = cdc_timer.latency() / 1000.0;
	float sha_latency = sha_timer.latency() / 1000.0;
	float dedup_latency = dedup_timer.latency() / 1000.0;
	float lzw_latency = lzw_timer.latency() / 1000.0;

	float input_throughput = (bytes_written * 8 / 1000000.0) / ethernet_latency; // Mb/s
	float encode_throughput = (total_input_bytes * 8 / 1000000.0) / encode_latency;
	float cdc_throughput = (total_input_bytes * 8 / 1000000.0) / cdc_latency;
	float sha_throughput = (total_input_bytes * 8 / 1000000.0) / sha_latency;
	float dedup_throughput = (total_input_bytes * 8 / 1000000.0) / dedup_latency;
	float lzw_throughput = (total_input_bytes * 8 / 1000000.0) / lzw_latency;

	float compression_ratio = 100.0 * bytes_written / total_input_bytes;

	// std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
	// 		<< " (Latency: " << ethernet_latency << "s)." << std::endl;
	std::cout << "\n===============================================" << std::endl;
	std::cout << "           Performance Statistics" << std::endl;
	std::cout << "===============================================" << std::endl;
	std::cout << "Total input size: " << total_input_bytes << " bytes" << std::endl;
	std::cout << "Total output size: " << bytes_written << " bytes" << std::endl;
	std::cout << "Compression ratio: " << compression_ratio << "%" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;

	std::cout << "\nLatencies:" << std::endl;
	std::cout << "  Ethernet:       " << ethernet_latency << " s" << std::endl;
	std::cout << "  Encode (total): " << encode_latency << " s" << std::endl;
	std::cout << "  CDC:            " << cdc_latency << " s" << std::endl;
	std::cout << "  SHA:            " << sha_latency << " s" << std::endl;
	std::cout << "  Dedup:          " << dedup_latency << " s" << std::endl;
	std::cout << "  LZW:            " << lzw_latency << " s" << std::endl;

	std::cout << "\nThroughputs (based on input data):" << std::endl;
	std::cout << "  Input Throughput:  " << input_throughput << " Mb/s" << std::endl;
	std::cout << "  Encode Throughput: " << encode_throughput << " Mb/s" << std::endl;
	std::cout << "  CDC Throughput:    " << cdc_throughput << " Mb/s" << std::endl;
	std::cout << "  SHA Throughput:    " << sha_throughput << " Mb/s" << std::endl;
	std::cout << "  Dedup Throughput:  " << dedup_throughput << " Mb/s" << std::endl;
	std::cout << "  LZW Throughput:    " << lzw_throughput << " Mb/s" << std::endl;

	std::cout << "\nTime Breakdown (% of encode time):" << std::endl;
	std::cout << "  CDC:   " << (cdc_latency / encode_latency * 100) << "%" << std::endl;
	std::cout << "  SHA:   " << (sha_latency / encode_latency * 100) << "%" << std::endl;
	std::cout << "  Dedup: " << (dedup_latency / encode_latency * 100) << "%" << std::endl;
	std::cout << "  LZW:   " << (lzw_latency / encode_latency * 100) << "%" << std::endl;

	std::cout << "\nChunking Statistics:" << std::endl;
	std::cout << "  Total LZW chunks created: " << lzw_chunk_count << std::endl;
	std::cout << "  Average chunk size: " << (lzw_chunk_count > 0 ? total_input_bytes / lzw_chunk_count : 0) << " bytes" << std::endl;

	std::cout << "===============================================\n" << std::endl;



	freeHashMap(global_hash_table);
	free(global_hash_table);

	return 0;

}

void encode(unsigned char * block, int block_length, int done, cl::CommandQueue q, cl::Kernel krnl_lzw,
            cl::Context context, cl_int* err) {
    memcpy(hold + hold_index, block, block_length);
    int block_length_new = block_length + hold_index;

    int * chunk_indices = (int *)malloc(sizeof(int) * BLOCKSIZE);
    int num_chunks = 0;

    cdc_timer.start();
    cdc(hold, block_length_new, chunk_indices, &num_chunks);
    cdc_timer.stop();

    int chunk_start = 0;

    int chunks_to_process = done ? num_chunks : (num_chunks > 0 ? num_chunks - 1 : 0);

    // int flag = done > 0 ? 1 : 0;


    for (int i = 0; i < chunks_to_process; i++) {
        int chunk_end = chunk_indices[i];
        int chunk_len = chunk_end - chunk_start;
        // int chunk_len = chunk_indices[i+1] - chunk_indices[i];
        unsigned char * hash_num = (unsigned char *)malloc(sizeof(unsigned char) * 32);
        sha_timer.start();
        sha(&hold[chunk_start], chunk_len, hash_num);
        sha_timer.stop();

        // dedup
        // hash_index is -1 if its a duplicate
        // otherwise it returns the index of the chunk that was encoded before
        dedup_timer.start();
        int hash_index = search(global_hash_table, hash_num);
        dedup_timer.stop();


        // dedup(hash_num, &hash_index, hash_map_t global_hash_table); // hash_index is -1 if its a duplicate
        // no more dedup

        if (hash_index == -1) {
            // new chunk - compress it
            // uint16_t * compressed = (uint16_t*)malloc(sizeof(uint16_t) * BLOCKSIZE);
            //unsigned char * compressed = (unsigned char*)malloc(sizeof(unsigned char) * BLOCKSIZE * 2);

            // Node tree[4096];
            //cl_int compressed_length = 0;

        //Create Buffers and initialize test values
            lzw_timer.start();
            cl::Buffer compressed_length(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, err); // kernel reads inputs from a?

            cl::Buffer chunk_buf(context, CL_MEM_READ_WRITE, chunk_len * sizeof(unsigned char), NULL, err); // kernel reads inputs from a?

            cl::Buffer compressed_buf (context, CL_MEM_READ_WRITE, chunk_len * sizeof(cl_uchar), NULL, err); // kernel write outputs from b?

            cl_int *compressed_length_ptr = (cl_int *)q.enqueueMapBuffer(compressed_length, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int));
            unsigned char *chunk_ptr = (unsigned char*)q.enqueueMapBuffer(chunk_buf, CL_TRUE, CL_MAP_WRITE, 0,  chunk_len * sizeof(unsigned char)); // we write inputs to a?
            cl_uchar *compressed_ptr = (cl_uchar *)q.enqueueMapBuffer(compressed_buf, CL_TRUE, CL_MAP_READ, 0, chunk_len * sizeof(cl_uchar));


            for(int i= 0; i < chunk_len; i++) {
            chunk_ptr[i] = hold[chunk_start + i];
            }
             // we read outputs from b?

            krnl_lzw.setArg(0, chunk_buf);
            krnl_lzw.setArg(1, chunk_len);
            krnl_lzw.setArg(2, compressed_buf);
            krnl_lzw.setArg(3, compressed_length);

            std::vector<cl::Event> exec_events, write_events;
            cl::Event write_ev, exec_ev, read_ev;

           // q.enqueueMigrateMemObjects({a_buf[i]}, 0 /* 0 means from host*/, write_deps.empty() ? nullptr : &write_deps, &write_ev);

            q.enqueueMigrateMemObjects({chunk_buf}, 0 /* 0 means from host*/, NULL, &write_ev);

            write_events.push_back(write_ev);
            q.enqueueTask(krnl_lzw, &write_events, &exec_ev);
            exec_events.push_back(exec_ev);
            q.enqueueMigrateMemObjects({compressed_buf},CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
            //lzw(&hold[chunk_start], chunk_len, compressed, &compressed_length);
            q.finish();
            lzw_timer.stop();

            uint32_t compressed_header = *compressed_length_ptr << 1;
            compressed_header &= ~(1);
            memcpy(&file[offset], &compressed_header, sizeof(compressed_header));
            offset += sizeof(compressed_header);

            memcpy(&file[offset], compressed_ptr, *compressed_length_ptr);
            offset += *compressed_length_ptr;

            insert(global_hash_table, hash_num, lzw_chunk_count);
            lzw_chunk_count++;

            //free(compressed);
        } else {
            uint32_t header = hash_index << 1;
            header |= 1;

            memcpy(&file[offset], &header, sizeof(header));
            offset += sizeof(header);
        }
        chunk_start = chunk_end;

        free(hash_num);
    }

    if (!done) {
        int rem_bytes = block_length_new - chunk_start;
        memmove(hold, hold + chunk_start, rem_bytes);
        hold_index = rem_bytes;
    } else {
        hold_index = 0;
    }

    free(chunk_indices);
    }



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
