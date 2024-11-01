/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */

#include "load_balancer.h"
#include "server.h"

load_balancer_t *init_load_balancer(bool enable_vnodes) {
    if (enable_vnodes)
        return NULL;
    load_balancer_t *load_balancer = malloc(sizeof(load_balancer_t));
    DIE(!load_balancer, "Memory allocation error\n");

    load_balancer->hash_function_docs = hash_string;
    load_balancer->hash_function_servers = hash_uint;

    // Create the hash_ring
    load_balancer->hash_ring = dll_create(sizeof(server_t));

    return load_balancer;
}

void loader_add_server(load_balancer_t* main, int server_id, int cache_size) {
    server_t *new_server = init_server(cache_size);
    new_server->id = server_id;

    // Add new server on hash ring
    dll_node_t *current_node = main->hash_ring->head;
    unsigned int idx = 0;

    // Find position
    while (idx < main->hash_ring->size &&
           main->hash_function_servers(&(*(server_t *)(current_node->data)).id)
           < main->hash_function_servers(&new_server->id)) {
        idx++;
        current_node = current_node->next;
    }
    // Add the new server
    dll_add_nth_node(main->hash_ring, idx, new_server);
    free(new_server);

    // Daca nu mai am alte servere in hash_ring
    if (main->hash_ring->size <= 1)
        return;

    // Redistribute the docs stored on the next server
    dll_node_t *next_node = current_node->next;

    // Empty the task queue
    queue_t *task_queue = (*(server_t *)next_node).tasks;
    while (!q_is_empty(task_queue)) {
        request *req = (request *)q_front(task_queue);
        server_handle_request((server_t *)next_node, req);
        q_dequeue(task_queue);
        free(req);
    }

    // Restore the docs
    server_t *next_server = (server_t *)next_node;
    dll_node_t *current_doc = next_server->database->head;
    idx = 0;
    unsigned int hash_server_id = main->hash_function_servers(&server_id);

    while (idx < next_server->database->size) {
        doc_t *document = (doc_t *)current_doc;
        // Daca trebuie mutat
        if (hash_string(document->doc_name) < hash_server_id) {
            // Elimin din database din sursa
            dll_node_t *removed_node =
            dll_remove_nth_node(next_server->database, idx);
            // Elimin din cache-ul sursei, daca exista acolo
            if (lru_cache_get(next_server->cache,
                              (*(doc_t *)removed_node).doc_name))
                lru_cache_remove(next_server->cache,
                                 (*(doc_t *)removed_node).doc_name);

            // Caut index-ul in database-ul din servewrul nou, ordonat dupa hash
            unsigned int pos = 0;
            dll_node_t *current_node = new_server->database->head;
            while (pos < new_server->database->size) {
                if (hash_string((*(doc_t *)current_node).doc_name) >
                    hash_string((*(doc_t *)removed_node).doc_name))
                    break;
                pos++;
                current_node = current_node->next;
            }
            // Adaug in database-ul destinatiei
            dll_add_nth_node_doc(new_server->database, pos, removed_node->data);
            free(removed_node);
        } else {
            idx++;
            current_doc = current_doc->next;
        }
    }
}

void loader_remove_server(load_balancer_t* main, int server_id) {
    dll_node_t *current_server = main->hash_ring->head;
    unsigned int found  = 0, idx = 0;

    while (idx < main->hash_ring->size && !found) {
        if ((*(server_t *)current_server).id == server_id) {
            found = 1;
            break;
        }
        idx++;
        current_server = current_server->next;
    }

    if (!found)
        return;

    server_t *found_server = (server_t *)current_server;

    // Execut task-urile ramase
    while (!q_is_empty(found_server->tasks)) {
        request *req = (request *)q_front(found_server->tasks);
        server_handle_request(found_server, req);
        q_dequeue(found_server->tasks);
    }

    // Mut documentele
    free_lru_cache(&found_server->cache);
    dll_node_t *current_doc = found_server->database->head;

    while (found_server->database->size) {
        current_doc = current_doc->next;
        dll_node_t *removed_doc = dll_remove_nth_node(found_server->database,
                                                        0);
        unsigned int doc_hashed =
        main->hash_function_docs((*(doc_t *)removed_doc).doc_name);

        // Caut index-ul pe care sa il adaug in database-ul urmatrului document
        unsigned int d_idx = 0;
        dll_node_t *next_s_doc =
        (*(server_t *)(current_server->next)).database->head;

        while (main->hash_function_docs((*(doc_t *)next_s_doc).doc_name)
               < doc_hashed)
            d_idx++;
        // Mut data
        dll_add_nth_node((*(server_t *)(current_server->next)).database, d_idx,
                         removed_doc->data);
        // Free doc
        free(removed_doc);
    }

    dll_node_t *removed_server = dll_remove_nth_node(main->hash_ring, idx);
    free(removed_server);
}

response *loader_forward_request(load_balancer_t* main, request *req) {
    response *res = NULL;

    dll_node_t *current_node = main->hash_ring->head;
    unsigned int idx = 0;
    while (idx < main->hash_ring->size &&
           main->hash_function_docs(req->doc_name) <
           main->hash_function_servers(&(*(server_t *)current_node->data).id))
            {
                current_node = current_node->next;
                idx++;
            }

    res = server_handle_request((server_t *)current_node->data, req);

    return res;
}

void free_load_balancer(load_balancer_t** main) {
    if (!main || !*main)
        return;

    server_t *s = (server_t *)(*main)->hash_ring->head->data;
    free_server(&s);

    free((*main)->hash_ring->head);

    free((*main)->hash_ring);

    free(*main);

    *main = NULL;
}
