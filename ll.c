/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#include "ll.h"
#include "server.h"

linked_list_t *ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(linked_list_t));
    DIE(list == NULL, "Cannot allocate memory for list\n");

    list->head = NULL;
    list->size = 0;
    list->data_size = data_size;

    return list;
}

void ll_add_nth_node(linked_list_t *list, unsigned int n, const void *new_data)
{
    ll_node_t *current_node, *previous_node;

    if (list == NULL && list->data_size == 0)
        return;
    // Trunchiez dimensiunea (adaug la finalul lisatei)
    if (n > list->size)
        n = list->size;

    info *data_info = (info *)new_data;
    char *data_key = data_info->key;
    dll_node_t *data_val = data_info->value;
    doc_t *data_doc = (doc_t *)data_val->data;

    // Aloc memorie pentru noul nod
    ll_node_t *new_node = malloc(sizeof(*new_node));

    // Copiez data
    new_node->data = malloc(sizeof(info));
    info *inf = (info *)new_node->data;

	inf->key = malloc(DOC_NAME_LENGTH);
	memcpy(inf->key, data_key, strlen(data_key) + 1);

	inf->value = malloc(sizeof(dll_node_t));
    dll_node_t *val_node = (dll_node_t *)inf->value;
    val_node->data = malloc(sizeof(doc_t));
    doc_t *val_doc = (doc_t *)val_node->data;
    val_doc->doc_content = malloc(DOC_CONTENT_LENGTH);
    memcpy(val_doc->doc_content, data_doc->doc_content,
            strlen(data_doc->doc_content) + 1);
    val_doc->doc_name = malloc(DOC_NAME_LENGTH);
    memcpy(val_doc->doc_name, data_doc->doc_name,
            strlen(data_doc->doc_name) + 1);

    // Parcurg lista pana la pozitia n
    previous_node = NULL;
    current_node = list->head;

    for (unsigned int i = 0; i < n; i++) {
        previous_node = current_node;
        current_node = current_node->next;
    }

    // Adaug nodul
    new_node->next = current_node;
    if (previous_node == NULL)
        list->head = new_node;
    else
        previous_node->next = new_node;

    // Cresc dimensiunea listei
    list->size++;
}

void ll_add_nth_node_queue(linked_list_t *list, unsigned int n,
                            const void *new_data)
{
    ll_node_t *current_node, *previous_node;

    if (list == NULL && list->data_size == 0)
        return;
    // Trunchiez dimensiunea (adaug la finalul lisatei)
    if (n > list->size)
        n = list->size;

    // Aloc memorie pentru noul nod
    ll_node_t *new_node = malloc(sizeof(*new_node));
    new_node->data = malloc(sizeof(request));

    // Copiez data
    request *new_request = (request *)new_node->data;
    request *src_request = (request *)new_data;

    new_request->doc_name = malloc(DOC_NAME_LENGTH);
    memcpy(new_request->doc_name, src_request->doc_name,
		   strlen(src_request->doc_name) + 1);

	new_request->doc_content = malloc(DOC_CONTENT_LENGTH);
	memcpy(new_request->doc_content, src_request->doc_content,
		   strlen(src_request->doc_content) + 1);

    new_request->type = src_request->type;

    // Parcurg lista pana la pozitia n
    previous_node = NULL;
    current_node = list->head;

    for (unsigned int i = 0; i < n; i++) {
        previous_node = current_node;
        current_node = current_node->next;
    }

    // Adaug nodul
    new_node->next = current_node;
    if (previous_node == NULL)
        list->head = new_node;
    else
        previous_node->next = new_node;

    // Cresc dimensiunea listei
    list->size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n)
{
    ll_node_t *previous_node, *current_node;

    if (list == NULL)
        return NULL;

    if (n >= list->size)
        n = list->size - 1;

    if (n == 0) {
        current_node = list->head;
        list->head = list->head->next;
        list->size--;
        return current_node;
    }

    // Parcurg lista pana la pozitia n
    previous_node = NULL;
    current_node = list->head;

    for (int i = 0; i < (int)n; i++) {
        previous_node = current_node;
        current_node = current_node->next;
    }

    // Elimin legaturile
    previous_node->next = current_node->next;

    list->size--;

    return current_node;
}

unsigned int ll_get_size(linked_list_t* list)
{
    return list->size;
}

void ll_free(linked_list_t **pp_list)
{
    // Verificăm dacă pointerul la listă și lista în sine nu sunt deja nule
    if (pp_list == NULL || *pp_list == NULL) {
        return;
    }

    ll_node_t *current_node = (*pp_list)->head;
    ll_node_t *next_node;

    // Parcurgem lista și eliberam fiecare nod în mod recursiv
    while (current_node != NULL) {
        next_node = current_node->next;
        // free(current_node->data);
        // key_val_free_function(current_node->data);
        info *inf = (info *)current_node->data;
        free(inf->key);
        dll_node_t *node = (dll_node_t *)inf->value;
        doc_t *doc = (doc_t *)node->data;
        free(doc->doc_content);
        free(doc->doc_name);
        free(node->data);
        free(node);
        free(current_node->data);
        free(current_node);
        current_node = next_node;
    }

    free(*pp_list);
    *pp_list = NULL;
}

void ll_print_int(linked_list_t *list)
{
    ll_node_t *current_node;

    if (list == NULL)
        return;

    current_node = list->head;

    while (current_node != NULL) {
        printf("%d ", *(int *)((current_node->data)));
        current_node = current_node->next;
    }

    printf("\n");
}

void ll_print_string(linked_list_t *list)
{
    ll_node_t *current_node;

    if (list == NULL)
        return;

    current_node = list->head;

    while (current_node != NULL) {
        printf("%s ", (char *)current_node->data);
        current_node = current_node->next;
    }

    printf("\n");
}
