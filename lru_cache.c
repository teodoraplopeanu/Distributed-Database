/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

lru_cache_t *init_lru_cache(unsigned int cache_capacity) {
    lru_cache_t *lru_cache = malloc(sizeof(lru_cache_t));
    DIE(!lru_cache, "Memory allocation error\n");

    lru_cache->cache_capacity = cache_capacity;

    // Create the list
    lru_cache->list = dll_create(sizeof(doc_t));

    // Create the hashmap
    lru_cache->hashmap = ht_create(cache_capacity, hash_function_string,
                                   compare_function_strings,
                                   key_val_free_function);

    return lru_cache;
}

bool lru_cache_is_full(lru_cache_t *cache) {
    if (dll_get_size(cache->list) >= cache->cache_capacity)
        return true;
    return false;
}

void free_lru_cache(lru_cache_t **cache) {
    if (!cache || !*cache)
        return;

    dll_free_doc(&(*cache)->list);
    ht_free((*cache)->hashmap);
    free((*cache));
    *cache = NULL;
}

bool lru_cache_put(lru_cache_t *cache, void *key, void *value,
                   void **evicted_key) {
    // Daca exista deja cheia in cache, return false
    if (ht_get(cache->hashmap, key))
        return false;

    if (lru_cache_is_full(cache) == true) {
        // Memoria este plina, deci elimin cea mai veche intrare
        dll_node_t *evicted_node = dll_remove_nth_node(cache->list,
                                            cache->list->size - 1);
        *evicted_key = malloc(DOC_NAME_LENGTH);
        memcpy(*evicted_key, (*(doc_t *)evicted_node->data).doc_name,
                strlen((*(doc_t *)evicted_node->data).doc_name) + 1);
        ht_remove_entry(cache->hashmap, *evicted_key);
        free((*(doc_t *)evicted_node->data).doc_content);
        free((*(doc_t *)evicted_node->data).doc_name);
        free(evicted_node->data);
        free(evicted_node);
    }

    // Adaug noul element in cache
    doc_t *new_doc = malloc(sizeof(doc_t));
    new_doc->doc_name = malloc(DOC_NAME_LENGTH);
    memcpy(new_doc->doc_name, (char *)key, strlen((char *)key) + 1);
    new_doc->doc_content = malloc(DOC_CONTENT_LENGTH);
    memcpy(new_doc->doc_content, (char *)value, strlen((char *)value) + 1);
    dll_add_nth_node_doc(cache->list, 0, new_doc);

    free(new_doc->doc_content);
    free(new_doc->doc_name);
    free(new_doc);

    // Adaug in hashmap
    ht_put(cache->hashmap, key, DOC_NAME_LENGTH, cache->list->head,
            sizeof(dll_node_t));

    return true;
}

void *lru_cache_get(lru_cache_t *cache, void *key) {
    dll_node_t *node = cache->list->head;
    unsigned int cnt = 0;
    int found = 0;

    while (cnt < cache->list->size) {
        if (strcmp((*(doc_t *)node->data).doc_name, (char *)key) == 0) {
            found = 1;
            break;
        }
        node = node->next;
        cnt++;
    }

    if (found)
        return node->data;
    return NULL;
}

void lru_cache_remove(lru_cache_t *cache, void *key) {
    dll_node_t *node = ht_get(cache->hashmap, key);
    dll_node_t *removed_node = NULL;

    if (node) {
        // Sterg elementul din dll
        dll_node_t *current_node = cache->list->head;
        unsigned int idx_to_remove = 0;

        for (unsigned int i = 0; i < cache->list->size; i++) {
            idx_to_remove = i;
            if (strcmp((*(doc_t *)current_node->data).doc_name,
                (char *)key) == 0)
                break;
            current_node = current_node->next;
        }
        if (idx_to_remove < cache->list->size)
            removed_node = dll_remove_nth_node(cache->list, idx_to_remove);

        // Sterg elementul din hashmap
        ht_remove_entry(cache->hashmap,
                        (*(doc_t *)removed_node->data).doc_name);
    }
}
