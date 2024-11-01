/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#include "dll.h"

doubly_linked_list_t *dll_create(unsigned int data_size)
{
	doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
	DIE(list == NULL, "Memory allocation error\n");

	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;

	return list;
}

dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	dll_node_t *current_node = list->head;

	for(unsigned int i = 1; i < n; i++)
		current_node = current_node->next;

	return current_node;
}

void dll_add_nth_node_doc(doubly_linked_list_t *list,
						   unsigned int n, const void *new_data)
{
	dll_node_t *current_node, *previous_node;

	if (list == NULL && list->data_size == 0)
		return;

	// Trunchiez dimensiunea lui n
	if (n > list->size)
		n = list->size;

	// Allocate memory for new node
	dll_node_t *new_node = malloc(sizeof(*new_node));
	new_node->data = malloc(sizeof(doc_t));

	// Memcpy data
	doc_t *new_node_doc = (doc_t *)new_node->data;
	doc_t *new_data_doc = (doc_t *)new_data;

	new_node_doc->doc_name = malloc(DOC_NAME_LENGTH);
	memcpy(new_node_doc->doc_name, new_data_doc->doc_name,
		   strlen(new_data_doc->doc_name) + 1);

	new_node_doc->doc_content = malloc(DOC_CONTENT_LENGTH);
	memcpy(new_node_doc->doc_content, new_data_doc->doc_content,
		   strlen(new_data_doc->doc_content) + 1);

	if (list->head == NULL) {
		list->head = new_node;
		list->head->next = new_node;
		list->head->prev = new_node;
		list->size++;
		return;
	}

	// Get to position n
	previous_node = NULL;
	current_node = list->head;

	for (unsigned int i = 0; i < n; i++)
		current_node = current_node->next;
	previous_node = current_node->prev;

	new_node->next = current_node;
	new_node->prev = previous_node;
	current_node->prev = new_node;
	previous_node->next = new_node;

	if (n == 0)
		list->head = new_node;

	list->size++;
}

void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
					   const void *new_data)
{
	dll_node_t *current_node, *previous_node;

	if (list == NULL && list->data_size == 0)
		return;

	// Trunchiez dimensiunea lui n
	if (n > list->size)
		n = list->size;

	// Allocate memory for new node
	dll_node_t *new_node = malloc(sizeof(*new_node));
	new_node->data = malloc(list->data_size);

	// Memcpy data
	memcpy(new_node->data, new_data, list->data_size);

	if (list->head == NULL) {
		list->head = new_node;
		list->head->next = new_node;
		list->head->prev = new_node;
		list->size++;
		return;
	}

	// Get to position n
	previous_node = NULL;
	current_node = list->head;

	for (unsigned int i = 0; i < n; i++)
		current_node = current_node->next;
	previous_node = current_node->prev;

	new_node->next = current_node;
	new_node->prev = previous_node;
	current_node->prev = new_node;
	previous_node->next = new_node;

	if (n == 0)
		list->head = new_node;

	list->size++;
}

dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	/* TODO */
	dll_node_t *current_node, *previous_node;

	if (n >= list->size)
		n = list->size - 1;

	if (n == 0) {
		current_node = list->head;
		list->head->next->prev = list->head->prev;
		list->head->prev->next = list->head->next;
		list->head = list->head->next;
		list->size--;
		return current_node;
	}

	if (n == list->size - 1) {
		current_node = list->head->prev;
		list->head->prev = current_node->prev;
		current_node->prev->next = list->head;
		list->size--;
		return current_node;
	}

	// Parcurg lista pana la n
	previous_node = NULL;
	current_node = list->head;

	for (unsigned int i = 0; i < n; i++) {
		previous_node = current_node;
		current_node = current_node->next;
	}

	// Elimin links urile
	previous_node->next = current_node->next;
	current_node->next->prev = previous_node;

	list->size--;

	return current_node;
}

unsigned int dll_get_size(doubly_linked_list_t *list)
{
	return list->size;
}


void dll_free(doubly_linked_list_t **pp_list)
{
	if (pp_list == NULL || *pp_list == NULL) {
		return;
	}

	// Pointer pentru a itera prin lista si a elibera nodurile
	dll_node_t *current_node = (*pp_list)->head;

	// Iteram prin lista si eliberam fiecare nod
	while ((*pp_list)->size) {
		dll_node_t *next_node = current_node->next;
		free(current_node->data);
		free(current_node);
		current_node = next_node;
		(*pp_list)->size--;
	}

	// Setam pointerul la lista la NULL pentru a indica că lista a fost
	// eliberata complet
	free(*pp_list);
	*pp_list = NULL;
}

void dll_free_doc(doubly_linked_list_t **pp_list)
{
	// Verificam daca pointerul la lista si lista in sine nu sunt deja nule
	if (pp_list == NULL || *pp_list == NULL) {
		return;
	}

	// Pointer pentru a itera prin lista si a elibera nodurile
	dll_node_t *current_node = (*pp_list)->head;

	// Iterăm prin lista si eliberam fiecare nod
	while ((*pp_list)->size) {
		dll_node_t *next_node = current_node->next;

		doc_t *current_node_doc = (doc_t *)current_node->data;
		free(current_node_doc->doc_content);
		free(current_node_doc->doc_name);
		free(current_node->data);
		// Eliberam nodul curent
		free(current_node);
		// Trecem la urmatorul nod
		current_node = next_node;
		(*pp_list)->size--;
	}

	// Setam pointerul la lista la NULL pentru a indica ca lista a fost
	// eliberata complet
	free(*pp_list);
	*pp_list = NULL;
}

void dll_print_int_list(doubly_linked_list_t* list)
{
	dll_node_t *current_node;

	if (list == NULL)
		return;

	current_node = list->head;

	for (unsigned int i = 0; i < list->size; i++) {
		printf("%d ", *(int *)(current_node->data));
		current_node = current_node->next;
	}

	printf("\n");
}
