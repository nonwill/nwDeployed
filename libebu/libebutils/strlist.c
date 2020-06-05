/*                                                            -*- C -*-
 * Copyright (c) 2006  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strlist.h"

/*
 * Unexported functions.
 */
static String_List_Node *
string_list_find_node(String_List *list, const char *string);


/*
 * Initialize `list'.
 */
void
string_list_initialize(String_List *list)
{
    list->node_count = 0;
    list->head = NULL;
    list->tail = NULL;
}


/*
 * Finalize `list'.
 */
void
string_list_finalize(String_List *list)
{
    string_list_delete_all(list);
}


/*
 * Add `string' to `list'.
 * If `list' has already had `string', it doesn't do anything.
 */
int
string_list_add(String_List *list, const char *string)
{
    String_List_Node *new_node;

    if (string_list_find_node(list, string) != NULL)
	return 0;

    new_node = (String_List_Node *)malloc(sizeof(String_List_Node));
    if (new_node == NULL)
	return -1;

    new_node->string = (char *)malloc(strlen(string) + 1);
    if (new_node->string == NULL) {
	free(new_node);
	return -1;
    }
    strcpy(new_node->string, string);

    if (list->node_count == 0) {
	new_node->back = NULL;
	list->head = new_node;
    } else {
	new_node->back = list->tail;
	list->tail->next = new_node;
    }

    new_node->next = NULL;
    list->tail = new_node;
    list->node_count++;

    return 0;
}


/*
 * Delete `string' from `list'.
 * The function doesn't complain if `list' doesn't have `string'.
 */
void
string_list_delete(String_List *list, const char *string)
{
    String_List_Node *p;

    p = string_list_find_node(list, string);
    if (p == NULL)
	return;

    if (p->next != NULL)
	p->next->back = p->back;
    if (p->back != NULL)
	p->back->next = p->next;
    free(p->string);
    free(p);

    list->node_count--;
    if (list->node_count == 0) {
	list->head = NULL;
	list->tail = NULL;
    }
}


/*
 * Delete all strings in `list'.
 */
void
string_list_delete_all(String_List *list)
{
    String_List_Node *p = list->head;
    String_List_Node *next_p;

    while (p != NULL) {
	next_p = p->next;
	free(p->string);
	free(p);
	p = next_p;
    }

    list->node_count = 0;
    list->head = NULL;
    list->tail = NULL;
}


/*
 * Return true if `list' has `string'.
 */
int
string_list_find(String_List *list, const char *string)
{
    return (string_list_find_node(list, string) != NULL);
}


/*
 * Count the number of strings in `list'.
 */
int
string_list_count_node(String_List *list)
{
    return list->node_count;
}


/*
 * Return a pointer to the node with `string' in `list'.
 * Return NULL if `list' doesn't have `string'.
 */
static String_List_Node *
string_list_find_node(String_List *list, const char *string)
{
    String_List_Node *p;

    for (p = list->head; p != NULL; p = p->next) {
	if (strcmp(string, p->string) == 0)
	    return p;
    }

    return NULL;
}


