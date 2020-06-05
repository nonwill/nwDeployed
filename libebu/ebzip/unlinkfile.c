/*                                                            -*- C -*-
 * Copyright (c) 2008-2009  Kazuhiro Ito
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

#include "ebzip.h"

/*
 * Add a file to the list of unlinking files.
 * If it succeeds, 0 is returned.  Otherwise -1 is returned.
 */
int unlink_files_add(const char *file_name)
{
    if (string_list_add(&unlinking_files, file_name)) {
	if (!ebzip_quiet_flag) {
	    fprintf(stderr, _("%s: warning: memory exhausted, file %s is not to be unlinked\n"), invoked_name, file_name);
	}
	return -1;
    }

    return 0;
}

/*
 * Unlink files.
 */
void unlink_files()
{
    String_List_Node *p = unlinking_files.head;

    while (p != NULL) {
	if (unlink(p->string) < 0 && !ebzip_quiet_flag) {
	    fprintf(stderr, _("%s: warning: failed to unlink the file: %s\n"),
		    invoked_name, p->string);
	}
	p = p->next;
    }
}
