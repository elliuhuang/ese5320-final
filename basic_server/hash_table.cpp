#include "hash_table.h"

void setNode(hash_node_t* node, unsigned char* key, int value, hash_node_t* next) {
    node->key = key;
    node->value = value;
    node->next = next;
}

void initializeHashMap(hash_map_t* map) {
    map->capacity = 100;
    map->numElements = 0;

    map->arr = (hash_node_t**)malloc(sizeof(hash_node_t*) * 100);
}

int hashFunction(hash_map_t* map, unsigned char* key) {
    return *key % map->capacity;
}

void insert(hash_map_t* map, unsigned char* key) {
    int bucket = hashFunction(map, key);

    hash_node_t* new_node = (hash_node_t*)malloc(sizeof(hash_node_t));
    setNode(new_node, key, map->numElements, map->arr[bucket]);

    map->arr[bucket] = new_node;

    map->numElements++;
}

int compareHash(unsigned char * hash1, unsigned char* hash2) {
    for (int i = 0; i < 32; i++) {
        if (hash1[i] != hash2[i]) {
            return 0;
        }
    }
    
    return 1;
}

int search(hash_map_t* map, uint256_t* key) {
    int bucket = hashFunction(map, key);

    hash_node_t* node = map->arr[bucket];

    while (node != NULL) {
        if (compareHash(key, node->key)) {
            return node->value;
        }

        node = node->next;
    }

    return -1;
}