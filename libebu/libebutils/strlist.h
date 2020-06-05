/*
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

#ifndef STRLIST_H
#define STRLIST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * Type definitions.
 */
typedef struct String_List_Struct String_List; 
typedef struct String_List_Node_Struct String_List_Node; 

struct String_List_Struct {
    int node_count;
    String_List_Node *head;
    String_List_Node *tail;
};


struct String_List_Node_Struct {
    char *string;
    String_List_Node *next;
    String_List_Node *back;
};


/*
 * Function declarations.
 */
void
string_list_initialize(String_List *list);

void
string_list_finalize(String_List *list);

int
string_list_add(String_List *list, const char *string);

void
string_list_delete(String_List *list, const char *string);

void
string_list_delete_all(String_List *list);

int
string_list_find(String_List *list, const char *string);

int
string_list_count_node(String_List *list);

#endif /* not STRLIST_H */
