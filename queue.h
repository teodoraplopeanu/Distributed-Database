/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ll.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

typedef struct queue_t queue_t;
struct queue_t
{
	linked_list_t *list;
};

queue_t *q_create(unsigned int data_size);

unsigned int
q_get_size(queue_t *q);

unsigned int
q_is_empty(queue_t *q);

void *q_front(queue_t *q);

ll_node_t *q_dequeue(queue_t *q);

void q_enqueue(queue_t *q, void *new_data);

void q_clear(queue_t *q);

void q_free(queue_t *q);
