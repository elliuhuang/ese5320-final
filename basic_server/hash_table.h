#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct hash_node {
    unsigned char* key; // 32-byte hash
    int value;
    struct hash_node* next;
} hash_node_t;

typedef struct hash_table {
    int numElements, capacity;

    hash_node_t** arr;
} hash_map_t;

void setNode(hash_node_t* node, unsigned char* key, int value);

void initializeHashMap(hash_map_t* map);
int hashFunction(hash_map_t* map, unsigned char* key);
void insert(hash_map_t* map, unsigned char* key);
int compareHash(unsigned char * hash1, unsigned char * hash2);
int search(hash_map_t* map, unsigned char* key);
void freeHashMap(hash_map_t* map);
