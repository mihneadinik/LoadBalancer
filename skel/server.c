/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#define NMAX 100

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

server_memory* init_server_memory() {
	server_memory *server = malloc(sizeof(server_memory));  // allocating a new server
	DIE(server == NULL, "Error creating server");  // checking if we had enough memory on heap

	// initial settings for the server (number of buckets, initial size)
	server->hmax = NMAX;
	server->size = 0;

	server->buckets = malloc(server->hmax * sizeof(linked_list_t *));
	DIE(server->buckets == NULL, "Error allocating buckets");
	for (int i = 0; i < server->hmax; i++)
		server->buckets[i] = ll_create(sizeof(info_obj));  // each bucket is a linked list

	return server;
}

// I used the direct-chaining method to combat collisions
void server_store(server_memory* server, char* key, char* value) {
	DIE(server == NULL, "No server in store function");  // checking if I have a valid server
	int index_value = hash_function_string(key) % server->hmax;  // where I have to add the entry
	// If I already have this entry I just update its value
	if (server_has_key(server, key) == 1) {
		ll_node_t *curr = server->buckets[index_value]->head;
		while(compare_function_strings(key, ((info_obj *)(curr->data))->key) != 0)
			curr = curr->next;
		memcpy(((info_obj *)(curr->data))->value, value, strlen(value) - 1);
	} else {
		// otherwise I create a new entry
		info_obj add;

		// allocate memory for its fields
		add.key = malloc(strlen(key) - 1);
		DIE(add.key == NULL, "Error");
		add.value = malloc(strlen(value) - 1);
		DIE(add.value == NULL, "Error");

		// deep copy the data
		memcpy(add.key, key, strlen(key) - 1);
		memcpy(add.value, value, strlen(value) - 1);

		// increase the number of items in the server and add it to the bucket
		server->size++;
		ll_add_nth_node(server->buckets[index_value], 0, &add);
	}
}

void server_remove(server_memory* server, char* key) {
	DIE(server == NULL, "No server in server_remove");
	int index_value = hash_function_string(key) % server->hmax;  // the index from where I have to delete the entry
	int rem_nr = 0;  // the index in the list that I have to remove
	if (server_has_key(server, key) == 0)
		return;  // if the key doesn't exit, I don't have what to remove
	ll_node_t *curr = server->buckets[index_value]->head;
	while (curr != NULL)
	{
		// search for the desired element in the list and get its position
		rem_nr++;
		if (compare_function_strings(key, ((info_obj *)(curr->data))->key) == 0)
			break;
		curr = curr->next;
	}
	// remove the element and free its memory
	curr = ll_remove_nth_node(server->buckets[index_value], rem_nr);
	free(((info_obj *)(curr->data))->key);
	free(((info_obj *)(curr->data))->value);
	free(curr->data);
	free(curr);
	server->size--;
}

char* server_retrieve(server_memory* server, char* key) {
	DIE(server == NULL, "No server in server_retrieve");  // checking if I have a valid server
	int index_value = hash_function_string(key) % server->hmax;  // the index from where I have to retrieve the value
	ll_node_t *curr = server->buckets[index_value]->head;
	if (curr == NULL)
		return NULL;  // if the list is empty
	while (curr != NULL) {
		// searching for the desired entry
		if (compare_function_strings(key, ((info_obj *)(curr->data))->key) == 0)
			return ((info_obj *)(curr->data))->value;
		curr = curr->next;
	}
	return NULL;  // if I don't have any entries with that hash
}

void free_server_memory(server_memory* server) {
	DIE(server == NULL, "No server in free_server_memory");
	for (int i = 0; i < server->hmax; i++) {
		ll_node_t *curr;

		while (server->buckets[i]->size > 0) {
			curr == ll_remove_nth_node(server->buckets[i], 0);
			free(((info_obj *)(curr->data))->key);
			free(((info_obj *)(curr->data))->value);
			free(curr->data);
			free(curr);
		}
		free(server->buckets[i]);
	}
	free(server->buckets);
	free(server);
}

// function that returns 1 if the key exists in the server and 0 otherwise
int server_has_key(server_memory* server, char* key) {
	DIE(server == NULL, "No server in server_has_key");
	int index_value = hash_function_string(key) % server->hmax;
	ll_node_t *curr = server->buckets[index_value]->head;
	if (curr == NULL)
		return 0;
	while (curr != NULL) {
		if (compare_function_strings(key, ((info_obj *)(curr->data))->key) == 0)
			return 1;
		curr = curr->next;
	}
}
