#include "lzw.cpp"

int main() {
    // Test 1: Simple repeated pattern
    printf("Test 1: Repeated pattern\n");
    unsigned char test1[] = "AAAAAABBBBBCCCCCDDDDD";
    int len1 = strlen((char*)test1);
    unsigned char compressed1[MAX_CHUNK_SIZE];
    int compressed_len1;

    lzw(test1, len1, compressed1, &compressed_len1);
    printf("Original length: %d bytes\n", len1);
    printf("Compressed length: %d bytes\n", compressed_len1);
    printf("Compression ratio: %.2f%%\n\n", 100.0 * compressed_len1 / len1);

    // Test 2: Text with repetition
    printf("Test 2: Text with repetition\n");
    unsigned char test2[] = "the quick brown fox jumps over the lazy dog the quick brown fox";
    int len2 = strlen((char*)test2);
    unsigned char compressed2[MAX_CHUNK_SIZE];
    int compressed_len2;

    lzw(test2, len2, compressed2, &compressed_len2);
    printf("Original length: %d bytes\n", len2);
    printf("Compressed length: %d bytes\n", compressed_len2);
    printf("Compression ratio: %.2f%%\n\n", 100.0 * compressed_len2 / len2);

    // Test 3: Random data (should not compress well)
    printf("Test 3: Random data\n");
    unsigned char test3[1000];
    unsigned int seed = 12345;
    for (int i = 0; i < 1000; i++) {
        seed = seed * 1103515245 + 12345;
        test3[i] = (seed >> 16) & 0xFF;
    }
    unsigned char compressed3[MAX_CHUNK_SIZE];
    int compressed_len3;

    lzw(test3, 1000, compressed3, &compressed_len3);
    printf("Original length: 1000 bytes\n");
    printf("Compressed length: %d bytes\n", compressed_len3);
    printf("Compression ratio: %.2f%%\n\n", 100.0 * compressed_len3 / 1000);

    // Test 4: Highly repetitive data
    printf("Test 4: Highly repetitive data\n");
    unsigned char test4[4096];
    for (int i = 0; i < 4096; i++) {
        test4[i] = i % 10 + 'A';
    }
    unsigned char compressed4[MAX_CHUNK_SIZE];
    int compressed_len4;

    lzw(test4, 4096, compressed4, &compressed_len4);
    printf("Original length: 4096 bytes\n");
    printf("Compressed length: %d bytes\n", compressed_len4);
    printf("Compression ratio: %.2f%%\n\n", 100.0 * compressed_len4 / 4096);

    return 0;
}
