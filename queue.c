/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#include "queue.h"

queue_t *q_create(unsigned int data_size)
{
	queue_t *queue = malloc(sizeof(queue_t));
    queue->list = ll_create(data_size);
    return queue;
}

unsigned int q_get_size(queue_t *q)
{
	return q->list->size;
}

unsigned int q_is_empty(queue_t *q)
{
	return (q_get_size(q)== 0);
}

void *q_front(queue_t *q)
{
	return q->list->head->data;
}

ll_node_t *q_dequeue(queue_t *q)
{
    ll_node_t *removed_node;
    if (q->list->size >= 1)
        removed_node = ll_remove_nth_node(q->list, 0);
    return removed_node;
}

void q_enqueue(queue_t *q, void *new_data)
{
	ll_add_nth_node_queue(q->list, q->list->size, new_data);
}

void q_clear(queue_t *q)
{
    while (q->list->size)
        q_dequeue(q);
}

void q_free(queue_t *q)
{
    q_clear(q);
    ll_free(&q->list);
    free(q);
}
