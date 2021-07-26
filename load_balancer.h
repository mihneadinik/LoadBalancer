/* Copyright 2021 <> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"

struct server_info;
typedef struct server_info server_info;

struct load_balancer;
typedef struct load_balancer load_balancer;

load_balancer* init_load_balancer();

void free_load_balancer(load_balancer* main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter 
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the 
 * load across the servers. The chosen server ID will be returned 
 * using the last parameter.
 */
void loader_store(load_balancer* main, char* key, char* value, int* server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID 
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the 
 * value associated to the key. The server will return NULL in case 
 * the key does NOT exist in the system.
 */
char* loader_retrieve(load_balancer* main, char* key, int* server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica TAGs and it will
 * place them inside the hash ring. The neighbor servers will 
 * distribute some the objects to the added server.
 */
void loader_add_server(load_balancer* main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 */
void loader_remove_server(load_balancer* main, int server_id);

server_info* create_h_ring_entry(load_balancer* main, int tag_nr,
                            int server_id, server_memory* server);

server_info* src_add_server(load_balancer* main, server_info* info);

server_info* get_sv_behind(load_balancer* main, int server_tag);

void add_redistribute(load_balancer* main, server_info* empty,
                        server_info* full, server_info *before);

void object_redistribution(server_memory *empty_sv,
            server_memory *full_sv, ll_node_t *curr);

void shift_right(load_balancer* main, int poz);

void shift_left(load_balancer *main, int poz);

int server_search(load_balancer *main, unsigned int hash_key);

server_memory* server_remover(load_balancer* main, int server_id);

#endif  /* LOAD_BALANCER_H_ */
