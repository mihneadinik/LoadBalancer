/* Copyright 2021 <Dinica Mihnea-Gabriel 313CA> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

#define MAX_SIZE 3*(1e5)

// struct that will be added in the hash ring to easily identify a server
struct server_info {
	unsigned int hash;
	int server_id;
	int tag_server;
	server_memory *server;
};

struct load_balancer {
	// Maximum size of the hash ring
	unsigned int max_size;
	// Current number of elements (servers + copies)
	unsigned int elements;
	// Hash ring array
	server_info **h_ring;
};

unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *) a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer* init_load_balancer() {
	// Allocating the load balancer struct
	load_balancer *main = malloc(sizeof(load_balancer));
	DIE(main == NULL, "Error allocating load balancer");

	// Initialising its fields
	main->elements = 0;
	main->max_size = MAX_SIZE;

	// Allocating the array for the hash ring
	// (it stores pointers to server_info)
	main->h_ring = malloc(main->max_size * sizeof(server_info*));
	DIE(main->h_ring == NULL, "Error allocating hash ring");
	for (unsigned int i = 0; i < main->max_size; i++)
		main->h_ring[i] = NULL;
	return main;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	DIE(main == NULL, "Error - no load balancer in store");

	// Getting the index where I have to add the object
	unsigned int hash_key = hash_function_key(key);
	int index = server_search(main, hash_key);

	*server_id = main->h_ring[index]->server_id;
	// Storing the object
	server_store(main->h_ring[index]->server, key, value);
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	DIE(main == NULL, "Error - no load balancer");

	// Getting the index where I should find the key
	unsigned int hash_key = hash_function_key(key);
	int index = server_search(main, hash_key);
	*server_id = ((server_info *)(main->h_ring[index]))->server_id;

	// Checking if the key exists
	return server_retrieve(((server_info *)(main->h_ring[index]))->server, key);
}

void loader_add_server(load_balancer* main, int server_id) {
	DIE(main == NULL, "Error - no load balancer in add_server");

	// Initialising the server
	server_memory *server = init_server_memory();

	server_info *info_0 = create_h_ring_entry(main, 0, server_id, server);
	server_info *info_1 = create_h_ring_entry(main, 1, server_id, server);
	server_info *info_2 = create_h_ring_entry(main, 2, server_id, server);

	// Adding to the hash ring and returning the server from which we
	// have to share objects
	server_info *server_neigh_0 = src_add_server(main, info_0);
	server_info *behind_0 = get_sv_behind(main, info_0->tag_server);
	add_redistribute(main, info_0, server_neigh_0, behind_0);
	server_info *server_neigh_1 = src_add_server(main, info_1);
	server_info *behind_1 = get_sv_behind(main, info_1->tag_server);
	add_redistribute(main, info_1, server_neigh_1, behind_1);
	server_info *server_neigh_2 = src_add_server(main, info_2);
	server_info *behind_2 = get_sv_behind(main, info_2->tag_server);
	add_redistribute(main, info_2, server_neigh_2, behind_2);
}

void loader_remove_server(load_balancer* main, int server_id) {
	DIE(main == NULL, "Error - no load balancer");

	server_memory *server_out;
	// The server id where an item will be redistributed
	int sv_red_id;

	// Remove all the 3 copies of a server
	for (int i = 0; i < 3; i++) {
		server_out = server_remover(main, server_id);
	}

	// Redistribute the items of a server
	for (unsigned int j = 0; j < server_out->hmax; j++) {
		ll_node_t *curr = server_out->buckets[j]->head;

		while(curr != NULL) {
			// Get the key-value pair
			char *key = ((info_obj *)(curr->data))->key;
			char *value = server_retrieve(server_out, key);

			loader_store(main, key, value, &sv_red_id);
			curr = curr->next;
		}
	}
	// Free the server
	free_server_memory(server_out);
}

void free_load_balancer(load_balancer* main) {
	DIE(main == NULL, "Error - no load balancer");
	while (main->elements > 0) {
		int server_id = ((server_info *)(main->h_ring[0]))->server_id;
		server_memory *server_out;

		// Remove all the 3 copies of a server
		for (int i = 0; i < 3; i++) {
			server_out = server_remover(main, server_id);
		}
		free_server_memory(server_out);
	}
	free(main->h_ring);
	free(main);
}

// Extra functions

// Returns the index where an item should be item
int server_search(load_balancer *main, unsigned int hash_key) {
	DIE(main == NULL, "Error - no load balancer in add_server");
	unsigned int index = 0, ok = 1;

	// Searching for the desired server in the ring
	while (index < main->elements && ok) {
		if (hash_key < main->h_ring[index]->hash) {
			ok = 0;
			break;
		}
		index++;
	}
	// If I didn't find a server with a greater hash, than I have to
	// add to the 1st server
	if (ok == 1)
		index = 0;
	return index;
}

// Function that initializes the information about a copy
server_info* create_h_ring_entry(load_balancer *main, int tag_nr,
							int server_id, server_memory *server) {
	DIE(main == NULL, "Error - no load balancer");

	// Allocate structure for the server info structure
	server_info *info = malloc(sizeof(server_info));
	DIE(info == NULL, "Error allocating server_info");

	// Initialising its fields
	info->server_id = server_id;
	info->tag_server = tag_nr * 1e5 + server_id;
	info->server = server;
	info->hash = hash_function_servers(&info->tag_server);
	return info;
}

// Function that adds a new (copy of the) server to the hashring
server_info* src_add_server(load_balancer* main, server_info* info) {
	DIE(main == NULL, "Error - no load balancer");
	DIE(info == NULL, "Error - no load balancer");
	unsigned int index = 0, ok = 1;
	// Start looking for the optimum position of a server in the hash ring
	while (ok && index <= main->elements) {
		if (main->h_ring[index] != NULL) {
			// If this is not the good position keep looking
			if (main->h_ring[index]->hash < info->hash) {
				index++;
			} else {
				// If I found the good position I add it
				if (main->h_ring[index]->hash > info->hash) {
					shift_right(main, index);
					main->h_ring[index] = info;
					ok = 0;
				} else {
					// If the hashes are identical
					if (info->server_id < main->h_ring[index]->server_id) {
						shift_right(main, index);
						main->h_ring[index] = info;
						ok = 0;
					} else {
						if (info->server_id >
							main->h_ring[index]->server_id &&
							info->hash ==
							main->h_ring[index + 1]->hash) {
							index++;
						} else {
							shift_right(main, index);
							index++;
							main->h_ring[index] = info;
							ok = 0;
						}
					}
				}
				main->elements++;
			}
		} else {
			main->h_ring[index] = info;
			main->elements++;
			ok = 0;
		}
	}
	// Returning the neighbour so we can balance the load
	if (main->h_ring[index + 1] != NULL) {
		return main->h_ring[index + 1];
	} else {
		if (main->elements != 1)  // This means that is only one server in the ring
			return main->h_ring[0];  // sau index +1 % main-elements
		else
			return NULL;
	}
}

// Function that redistributes the elements when a new server is added
void add_redistribute(load_balancer* main, server_info *empty,
						server_info *full, server_info *before) {
	if (full == NULL || full->server_id == empty->server_id)
		return;
	server_memory *empty_sv = empty->server, *full_sv = full->server;
	// Check each object stored previously on the server
	for (unsigned int i = 0; i < full_sv->hmax; i++) {
		ll_node_t *curr = full_sv->buckets[i]->head;
		while (curr != NULL) {
			unsigned int key_hash =
			hash_function_string(((info_obj *)(curr->data))->key);

			// if the new server is the one after "0" value-point on the hashring
			if (main->h_ring[0]->tag_server == empty->tag_server) {
				if ((key_hash < empty->hash) || key_hash >= before->hash) {
					// restore the object if necessary
					ll_node_t *curr_cp = curr;
					curr = curr->next;
					object_redistribution(empty_sv, full_sv, curr_cp);
				} else {
					curr = curr->next;
				}
			} else {
				if (key_hash < empty->hash && key_hash >= before->hash) {
					// restore the object if necessary
					ll_node_t *curr_cp = curr;
					curr = curr->next;
					object_redistribution(empty_sv, full_sv, curr_cp);
				} else {
					curr = curr->next;
				}
			}
		}
	}
}

// Function that moves an object from a server to the other
void object_redistribution(server_memory *empty_sv, server_memory *full_sv,
														ll_node_t *curr) {
	// Get the key-value pair
	char *key = ((info_obj *)(curr->data))->key;
	char *value = ((info_obj *)(curr->data))->value;

	// Delete from the previous server and add to the new one
	server_store(empty_sv, key, value);
	server_remove(full_sv, key);
}

// Getting the (copy of the) server behind our current position
server_info* get_sv_behind(load_balancer* main, int server_tag) {
	DIE(main == NULL, "Error - no load balancer in add_server");

	server_info *server_out;
	unsigned int index = 0, ok = 1;
	while (ok && index < main->elements) {
		// looking for the actual server to return its back-neighbour
		if (((server_info *)(main->h_ring[index]))->tag_server == server_tag) {
			if (index == 0)
				// if my server is on the 1st position, its back-neighbour
				// is the last server on the hashring
				server_out = ((server_info *)(main->h_ring[main->elements - 1]));
			else
				server_out = ((server_info *)(main->h_ring[index - 1]));
			ok = 0;
		} else {
			index++;
		}
	}
	return server_out;
}

// Shifting the hashring with one position to the right
void shift_right(load_balancer* main, int poz) {
	for (int i = main->elements + 1; i > poz; i--) {
		main->h_ring[i] = main->h_ring[i - 1];
	}
}

// Shifting the hashring with one position to the left
void shift_left(load_balancer *main, int poz) {
	DIE(main == NULL, "Error - no load balancer");
	for (unsigned int i = poz; i < main->elements - 1; i++) {
		main->h_ring[i] = main->h_ring[i + 1];
	}
	main->h_ring[main->elements - 1] = NULL;
	main->elements--;
}

// Removing a copy from the hashring
server_memory* server_remover(load_balancer* main, int server_id) {
	DIE(main == NULL, "Error - no load balancer");

	unsigned int index = 0, ok = 1;
	server_memory *server_out;
	while (ok && index < main->elements) {
		if (main->h_ring[index]->server_id == server_id) {
			ok = 0;
			server_out = main->h_ring[index]->server;
			free(main->h_ring[index]);
			shift_left(main, index);
		} else {
			index++;
		}
	}
	return server_out;
}
