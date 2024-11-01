/*
 * Copyright (c) 2024, Teodora Plopeanu <teodora.plopeanu@stud.acs.upb.ro>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

static response
*server_edit_document(server_t *s, char *doc_name, char *doc_content) {
	response *res = malloc(sizeof(response));
	DIE(!res, "Memory allocation error!\n");

	res->server_id = s->id;

	// Cache has doc
	if (lru_cache_get(s->cache, doc_name)) {
		res->server_log = malloc(MAX_LOG_LENGTH);
		DIE(!res->server_log, "Memory allocation error\n");
		sprintf(res->server_log, LOG_HIT, doc_name);
		res->server_response = malloc(MAX_RESPONSE_LENGTH);
		DIE(!res->server_response, "Memory allocation error\n");
		sprintf(res->server_response, MSG_B, doc_name);

		// Modify doc_content in cache->list
		dll_node_t *current_node = s->cache->list->head;
		int found  = 0;
		for (unsigned int i = 0; i < s->cache->list->size && !found; i++) {
			if (strcmp((*(doc_t *)current_node->data).doc_name, doc_name) == 0) {
				found = 1;
				break;
			}
			current_node = current_node->next;
		}
		if (found == 1)
			memcpy((*(doc_t *)current_node->data).doc_content, doc_content,
				   strlen(doc_content) + 1);

		// Modify content in database
		dll_node_t *current_node2 = s->database->head;
		int found2  = 0;
		for (unsigned int i = 0; i < s->database->size && !found2; i++) {
			if (strcmp((*(doc_t *)current_node2->data).doc_name, doc_name) == 0) {
				found2 = 1;
				break;
			}
			current_node2 = current_node2->next;
		}
		if (found == 1)
			memcpy((*(doc_t *)current_node2->data).doc_content, doc_content,
				   strlen(doc_content) + 1);

	} else {
		// Try to find the key in database
		dll_node_t *current_node = s->database->head;
		int found  = 0;
		for (unsigned int i = 0; i < s->database->size && !found; i++) {
			if (strcmp((*(doc_t *)current_node->data).doc_name, doc_name) == 0) {
				found = 1;
				break;
			}
			current_node = current_node->next;
		}

		res->server_response = malloc(MAX_RESPONSE_LENGTH);
		DIE(!res->server_response, "Memory allocation error\n");
		// Database has doc
		if (found) {
			sprintf(res->server_response, MSG_B, doc_name);
			// Editez continutul
			memcpy((*(doc_t *)current_node->data).doc_content, doc_content,
				   strlen(doc_content) + 1);
		} else {
			// Doc does not exist on server
			sprintf(res->server_response, MSG_C, doc_name);
			// Add doc on database
			doc_t *doc = malloc(sizeof(doc_t));
			doc->doc_content = doc_content;
			doc->doc_name = doc_name;
			dll_add_nth_node_doc(s->database, 0, doc);
			free(doc);
		}

		res->server_log = malloc(MAX_LOG_LENGTH);
		DIE(!res->server_log, "Memory allocation error\n");
		int full = 0;
		if (lru_cache_is_full(s->cache))
			full = 1;

		void *evicted_key = NULL;
		lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);

		if (full) {
			sprintf(res->server_log, LOG_EVICT, doc_name,
					(char *)evicted_key);
			free(evicted_key);
		} else {
			sprintf(res->server_log, LOG_MISS, doc_name);
		}
	}

	return res;
}

static response
*server_get_document(server_t *s, char *doc_name) {
	response *res = malloc(sizeof(response));
	DIE(!res, "Memory allocation error\n");

	res->server_id = s->id;

	// Doc is in cache
	if (lru_cache_get(s->cache, doc_name)) {
		res->server_log = malloc(MAX_LOG_LENGTH);
		DIE(!res->server_log, "Memory allocation error\n");
		sprintf(res->server_log, LOG_HIT, doc_name);
		res->server_response = malloc(MAX_RESPONSE_LENGTH);
		DIE(!res->server_response, "Memory allocation error\n");
		doc_t *ptr = (doc_t *)lru_cache_get(s->cache, doc_name);
		sprintf(res->server_response, "%s", ptr->doc_content);
	} else {
		// Try to find the node in database
		dll_node_t *current_node = s->database->head;
		int found  = 0;
		for (unsigned int i = 0; i < s->database->size && !found; i++) {
			if (strcmp((*(doc_t *)current_node->data).doc_name, doc_name)
				== 0) {
				found = 1;
				break;
			}
			current_node = current_node->next;
		}

		// Doc is in database
		if (found) {
			res->server_response = malloc(MAX_RESPONSE_LENGTH);\
			DIE(!res->server_response, "Memory allocation error\n");
			sprintf(res->server_response, "%s",
					(*(doc_t *)current_node->data).doc_content);

			// Cache is full
			int full = 0;
			if (lru_cache_is_full(s->cache))
				full = 1;

			void *evicted_key = NULL;
			// Add doc to cache
			lru_cache_put(s->cache, doc_name,
						  (*(doc_t *)current_node).doc_content, &evicted_key);

			res->server_log = malloc(MAX_LOG_LENGTH);
			DIE(!res->server_log, "Memroy allocation error\n");
			if (full) {
				sprintf(res->server_log, LOG_EVICT, doc_name,
						(char *)evicted_key);
				free(evicted_key);
			} else {
				// Cache is not full
				sprintf(res->server_log, LOG_MISS, doc_name);
			}

		} else {
			// Doc does not exist on the server
			res->server_log = malloc(MAX_LOG_LENGTH);
			DIE(!res->server_log, "Memory allocation error\n");
			sprintf(res->server_log, LOG_FAULT, doc_name);
			res->server_response = NULL;
		}
	}

	return res;
}

server_t *init_server(unsigned int cache_size) {
	server_t *server = malloc(sizeof(server_t));
	DIE(server == NULL, "Memory allocation error\n");

	server->cache_size = cache_size;

	server->cache = init_lru_cache(cache_size);
	DIE(!server->cache, "Memory allocation error\n");

	server->tasks = q_create(sizeof(request));
	DIE(!server->tasks, "Memory allocation error\n");

	server->database = dll_create(sizeof(doc_t));
	DIE(!server->database, "Memory allocation error\n");

	return server;
}

response *server_handle_request(server_t *s, request *req) {
	response *res = NULL;
	request *copy_req = malloc(sizeof(request));

	switch (req->type) {
		case GET_DOCUMENT:
			free(copy_req);
			goto empty_queue;
get:
			res = server_get_document(s, req->doc_name);
			goto end;

		case EDIT_DOCUMENT:
			copy_req->type = req->type;
			copy_req->doc_name = malloc(DOC_NAME_LENGTH);
			memcpy(copy_req->doc_name, req->doc_name, strlen(req->doc_name) + 1);
			copy_req->doc_content = malloc(DOC_CONTENT_LENGTH);
			memcpy(copy_req->doc_content, req->doc_content,
				   strlen(req->doc_content) + 1);
			q_enqueue(s->tasks, copy_req);
			free(copy_req->doc_content);
			free(copy_req->doc_name);
			free(copy_req);

			res = malloc(sizeof(response));
			DIE(!res, "Memory allocation error\n");
			res->server_id = s->id;
			res->server_log = malloc(MAX_LOG_LENGTH);
			DIE(!res->server_log, "Memory allocation error\n");
			sprintf(res->server_log, LOG_LAZY_EXEC, s->tasks->list->size);
			res->server_response = malloc(MAX_RESPONSE_LENGTH);
			DIE(!res->server_response, "Memory allocation error\n");
			sprintf(res->server_response, MSG_A,
					get_request_type_str(req->type), req->doc_name);
			goto end;

		default:
			goto end;
	}

empty_queue:
	while (!q_is_empty(s->tasks)) {
		request *req_new = (request *)q_front(s->tasks);

		response *edit_response = server_edit_document(s, req_new->doc_name,
													   req_new->doc_content);
		PRINT_RESPONSE(edit_response);
		ll_node_t *removed_ll_node = q_dequeue(s->tasks);
		free((*(request *)removed_ll_node->data).doc_content);
		free((*(request *)removed_ll_node->data).doc_name);
		free(removed_ll_node->data);
		free(removed_ll_node);
	}
goto get;

end:
	return res;
}

void free_doc_data(void *data)
{
	free((*(doc_t *)data).doc_name);
	free((*(doc_t *)data).doc_content);
	free(data);
}

void free_server(server_t **s) {
	if (!s || !*s)
		return;

	free_lru_cache(&(*s)->cache);
	dll_free_doc(&(*s)->database);
	q_free((*s)->tasks);
	free(*s);
	*s = NULL;
}
