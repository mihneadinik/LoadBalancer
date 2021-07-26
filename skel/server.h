/* Copyright 2021 <> */
#ifndef SERVER_H_
#define SERVER_H_

#include "LinkedList.h"

typedef struct server_memory server_memory;
typedef struct info_obj info_obj;

struct server_memory {
	linked_list_t **buckets;  // Array of linked lists
	unsigned int size;  // Current number of elements stored
	unsigned int hmax;  // Number of buckets
	// int (*compare_function)(void*, void*);  // Function that compares 2 keys
};

struct info_obj {
	char *key;
	char *value;
};

int compare_function_strings(void *a, void *b);

unsigned int hash_function_string(void *a);

server_memory* init_server_memory();

void free_server_memory(server_memory* server);

/**
 * server_store() - Stores a key-value pair to the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory* server, char* key, char* value);

/**
 * server_remove() - Removes a key-pair value from the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory* server, char* key);

/**
 * server_retrieve() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char* server_retrieve(server_memory* server, char* key);

int server_has_key(server_memory* server, char* key);

#endif  /* SERVER_H_ */
