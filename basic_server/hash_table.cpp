#include "hash_table.h"

void setNode(hash_node_t* node, unsigned char* key, int value, hash_node_t* next) {
    node->key = key;
    node->value = value;
    node->next = next;
}

void initializeHashMap(hash_map_t* map) {
    map->capacity = 4096;
    map->numElements = 0;

    map->arr = (hash_node_t**)malloc(sizeof(hash_node_t*) * 4096);
    for (int i = 0; i < map->capacity; i++) {
        map->arr[i] = NULL;
    }
}

int hashFunction(hash_map_t* map, unsigned char* key) {
    unsigned int hash = 0;
    hash = ((unsigned int)key[0] << 24) |
           ((unsigned int)key[1] << 16) |
           ((unsigned int)key[2] << 8) |
           ((unsigned int)key[3]);
    return hash % map->capacity;
}

void insert(hash_map_t* map, unsigned char* key) {
    int bucket = hashFunction(map, key);

    unsigned char* key_copy = (unsigned char*)malloc(sizeof(unsigned char) * 32);
    memcpy(key_copy, key, 32);

    hash_node_t* new_node = (hash_node_t*)malloc(sizeof(hash_node_t));
    setNode(new_node, key_copy, map->numElements, map->arr[bucket]);
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

int search(hash_map_t* map, unsigned char* key) {
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

void freeHashMap(hash_map_t* map) {
    for (int i = 0; i < map->capacity; i++) {
        hash_node_t* node = map->arr[i];
        while (node != NULL) {
            hash_node_t* temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(map->arr);
}
