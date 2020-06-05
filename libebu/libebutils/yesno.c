/*                                                            -*- C -*-
 * Copyright (c) 1998-2006  Motoyuki Kasahara
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
#include <sys/types.h>
#include <string.h>

#define YESNO_RESPONSE_SIZE	80

/*
 * Get y/n response to `message'.
 * If `y' is selected, 1 is returned.  Otherwise 0 is returned.
 */
int
query_y_or_n(const char *message)
{
    char response[YESNO_RESPONSE_SIZE];
    char *head, *tail;

    for (;;) {
	/*
	 * Output `message' to standard error.
	 */
	fputs(message, stderr);
	fflush(stderr);

	/*
	 * Get a line from standard in.
	 */
	if (fgets(response, YESNO_RESPONSE_SIZE, stdin) == NULL)
	    return 0;
	if (strchr(response, '\n') == NULL)
	    continue;

	/*
	 * Delete spaces and tabs in the beginning and end of the
	 * line.
	 */
        for (head = response; *head == ' ' || *head == '\t'; head++)
            ;
	for (tail = head + strlen(head) - 1;
	     head <= tail && (*tail == ' ' || *tail == '\t' || *tail == '\n');
	     tail--)
            *tail = '\0';

	/*
	 * Return if the line is `y' or `n'.
	 */
	if ((*head == 'Y' || *head == 'y') && *(head + 1) == '\0')
	    return 1;
	if ((*head == 'N' || *head == 'n') && *(head + 1) == '\0')
	    return 0;
    }

    /* not reached */
    return 0;
}
