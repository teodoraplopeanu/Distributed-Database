/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#ifndef DLL_H
#define DLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "constants.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

typedef struct doc_t {
	char *doc_name;
	char *doc_content;
} doc_t;

typedef struct dll_node_t dll_node_t;
struct dll_node_t
{
    void* data;
    dll_node_t *prev, *next;
};

typedef struct doubly_linked_list_t doubly_linked_list_t;
struct doubly_linked_list_t
{
    dll_node_t* head;
    unsigned int data_size;
    unsigned int size;
};


doubly_linked_list_t *dll_create(unsigned int data_size);

dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n);

void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
						const void *new_data);

void dll_add_nth_node_doc(doubly_linked_list_t *list, unsigned int n,
							const void *new_data);

dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n);

unsigned int dll_get_size(doubly_linked_list_t *list);

void dll_free(doubly_linked_list_t **pp_list);

void dll_free_doc(doubly_linked_list_t **pp_list);

void dll_print_int_list(doubly_linked_list_t *list);

#endif
