/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */
#include "ht.h"
#include "dll.h"

/*
 * Functii de comparare a cheilor:
 */
int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */

unsigned int hash_function_string(void *a)
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

/*
 * Functie apelata pentru a elibera memoria ocupata de cheia si valoarea unei
 * perechi din hashtable. Daca cheia sau valoarea contin tipuri de date complexe
 * aveti grija sa eliberati memoria luand in considerare acest aspect.
 */
void key_val_free_function(void *data) {
	/* TODO */
    free(((info*)data)->key);
    // free(((info*)data)->value);
	dll_node_t *node = (dll_node_t *)(((info*)data)->value);
	doc_t *doc = (doc_t *)node->data;
	free(doc->doc_content);
	free(doc->doc_name);
	free(node->data);
	free(node);
	free(data);
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	/* TODO */
    hashtable_t *ht = malloc(sizeof(hashtable_t));
    DIE(!ht, "Memory allocation error\n");

    ht->buckets = malloc(hmax * sizeof(linked_list_t *));
    DIE(!ht->buckets, "Memory allocation error\n");

    for (unsigned int i = 0; i < hmax; ++i)
        ht->buckets[i] = ll_create(sizeof(info));

    ht->size = 0;
    ht->hmax = hmax;
    ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;

	return ht;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable
 * folosind functia put;
 * 0, altfel.
 */
int ht_has_key(hashtable_t *ht, void *key)
{
	/* TODO */
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[index];

	ll_node_t *current_node = bucket->head;

	while (current_node != NULL) {
		void *current_key = ((info *)current_node->data)->key;
		if (ht->compare_function(current_key, key) == 0)
			return 1;
		current_node = current_node->next;
	}

	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	/* TODO */
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[index];

	ll_node_t *current_node = bucket->head;

	while(current_node != NULL) {
		void *current_key = ((info*)current_node->data)->key;
		if (ht->compare_function(current_key, key) == 0)
			return ((info*)current_node->data)->value;
		current_node = current_node->next;
	}

	return NULL;
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune
 * tipul ei), in momentul in care se creeaza o noua intrare in hashtable (in
 * cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie a
 * valorii la care pointeaza key si adresa acestei copii trebuie salvata in
 * structura info asociata intrarii din ht. Pentru a sti cati octeti trebuie
 * alocati si copiati, folositi parametrul key_size.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel
 * put(ht, key_actual, value_actual), valoarea la care pointeaza key_actual
 * poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia
 * unei intrari din hashtable. Nu ne dorim acest lucru, fiindca exista riscul sa
 * ajungem in situatia in care nu mai stim la ce cheie este inregistrata o
 * anumita valoare.
 */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	int index = ht->hash_function(key) % ht->hmax;

	linked_list_t *bucket = ht->buckets[index];
	ll_node_t *current_node = bucket->head;

	while (current_node != NULL) {
		void *current_key = ((info*)current_node->data)->key;
		if (ht->compare_function(current_key, key) == 0) {
			void *current_value = ((info*)current_node->data)->value;
			free(current_value);
			((info*)current_node)->value = malloc(value_size);
			// dll_node_t *node = (dll_node_t *)
			memcpy(((info*)current_node->data)->value, value, value_size);
			return;
		}
		current_node = current_node->next;
	}

	// info new_info;
	// new_info.key = malloc(key_size);
	// memcpy(new_info.key, key, key_size);
	// new_info.value = malloc(value_size);
	// // memcpy(new_info.value, value, value_size);
	// dll_node_t *node = (dll_node_t *)new_info.value;
	info *new_info = malloc(sizeof(info));

	new_info->key = malloc(key_size);
	memcpy(new_info->key, key, key_size);

	new_info->value = malloc(value_size);
	dll_node_t *node = (dll_node_t *)new_info->value;
	node->data = malloc(sizeof(doc_t));
	doc_t *doc = (doc_t *)node->data;
	doc->doc_content = malloc(DOC_CONTENT_LENGTH);
	doc->doc_name = malloc(DOC_NAME_LENGTH);
	memcpy(doc->doc_name, ((*(doc_t *)(*(dll_node_t *)value).data).doc_name),
			strlen(((*(doc_t *)(*(dll_node_t *)value).data).doc_name)) + 1);
	memcpy(doc->doc_content, ((*(doc_t *)(*(dll_node_t *)value).data).doc_content),
			strlen(((*(doc_t *)(*(dll_node_t *)value).data).doc_content)) + 1);

	ll_add_nth_node(bucket, 0, new_info);
	free(doc->doc_content);
	free(doc->doc_name);
	free(node->data);
	free(new_info->value);
	free(new_info->key);
	free(new_info);
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o
 * intrare din hashtable (adica memoria pentru copia lui key --vezi observatia
 * de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
void ht_remove_entry(hashtable_t *ht, void *key)
{
	/* TODO */
	int index = ht->hash_function(key) % ht->hmax;

	linked_list_t *bucket = ht->buckets[index];
	ll_node_t *current_node = bucket->head;

	int n = 0;

	while (current_node != NULL) {
		void *current_key = ((info*)current_node->data)->key;
		if (ht->compare_function(current_key, key) == 0) {
			break;
		}
		n++;
		current_node = current_node->next;
	}

	ll_node_t *removed_node = ll_remove_nth_node(bucket, n);
	key_val_free_function(removed_node->data);
	free(removed_node);
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable,
 * dupa care elibereaza si memoria folosita pentru a stoca structura hashtable.
 */
void ht_free(hashtable_t *ht)
{
	for (unsigned int i = 0; i < ht->hmax; ++i)
	{
		linked_list_t *bucket = ht->buckets[i];
        if (bucket) {
            ll_node_t *current_node = bucket->head;
            while (current_node) {
                info *current_info = (info *)current_node->data;
                if (current_info) {
                    if (current_info->key) {
                        free(current_info->key);
                    }
                    if (current_info->value) {
						dll_node_t *node = (dll_node_t *)current_info->value;
						doc_t *doc_data = (doc_t *)node->data;
						free(doc_data->doc_content);
						free(doc_data->doc_name);
						free(node->data);
                        free(current_info->value);
                    }
                }
                ll_node_t *next_node = current_node->next;
                free(current_node->data);
                free(current_node);
                current_node = next_node;
            }
            free(bucket);
		}
	}

	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
