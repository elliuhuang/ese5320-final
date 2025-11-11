#include "cdc.cpp"

// Helper function to print chunks for debugging
void print_chunks(unsigned char *block, int *chunk_indices, int num_chunks) {
    printf("Found %d chunks:\n", num_chunks);
    int start = 0;
    for (int i = 0; i < num_chunks; i++) {
        int end = chunk_indices[i];
        int size = end - start;
        printf("Chunk %d: bytes %d-%d (size: %d bytes)\n", i, start, end-1, size);
        start = end;
    }
}

// Test function
int main() {
    // Test 1: Small block
    printf("Test 1: Small block (100 bytes)\n");
    unsigned char small_block[100];
    for (int i = 0; i < 100; i++) {
        small_block[i] = i % 256;
    }
    int chunks1[10];
    int num_chunks1;
    cdc(small_block, 100, chunks1, &num_chunks1);
    print_chunks(small_block, chunks1, num_chunks1);
    printf("\n");

    // Test 2: Medium block with repeating pattern
    printf("Test 2: Medium block (50KB)\n");
    int size2 = 50000;
    unsigned char *block2 = (unsigned char*)malloc(size2);
    for (int i = 0; i < size2; i++) {
        block2[i] = (i / 100) % 256;  // Pattern that changes every 100 bytes
    }
    int *chunks2 = (int*)malloc(size2 / MIN_CHUNK_SIZE * sizeof(int) * 2);
    int num_chunks2;
    cdc(block2, size2, chunks2, &num_chunks2);
    print_chunks(block2, chunks2, num_chunks2);

    // Calculate average chunk size
    int total_size = 0;
    int start = 0;
    for (int i = 0; i < num_chunks2; i++) {
        total_size += chunks2[i] - start;
        start = chunks2[i];
    }
    printf("Average chunk size: %d bytes\n", total_size / num_chunks2);
    free(block2);
    free(chunks2);
    printf("\n");

    // Test 3: Large block with random data
    printf("Test 3: Large block (1MB random data)\n");
    int size3 = 1024 * 1024;
    unsigned char *block3 = (unsigned char*)malloc(size3);
    // Fill with pseudo-random data
    unsigned int seed = 12345;
    for (int i = 0; i < size3; i++) {
        seed = seed * 1103515245 + 12345;
        block3[i] = (seed >> 16) & 0xFF;
    }
    int *chunks3 = (int*)malloc(size3 / MIN_CHUNK_SIZE * sizeof(int) * 2);
    int num_chunks3;
    cdc(block3, size3, chunks3, &num_chunks3);

    // Calculate statistics
    int min_size = MAX_CHUNK_SIZE;
    int max_size = 0;
    total_size = 0;
    start = 0;
    for (int i = 0; i < num_chunks3; i++) {
        int chunk_size = chunks3[i] - start;
        if (chunk_size < min_size) min_size = chunk_size;
        if (chunk_size > max_size) max_size = chunk_size;
        total_size += chunk_size;
        start = chunks3[i];
    }
    printf("Number of chunks: %d\n", num_chunks3);
    printf("Min chunk size: %d bytes\n", min_size);
    printf("Max chunk size: %d bytes\n", max_size);
    printf("Average chunk size: %d bytes\n", total_size / num_chunks3);

    free(block3);
    free(chunks3);

    return 0;
}
