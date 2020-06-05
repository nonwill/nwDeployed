/*
 * Copyright (c) 2000-2006  Motoyuki Kasahara
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

#ifndef URLPARTS_H
#define URLPARTS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * URL Parts.
 */
typedef struct {
    char *url;
    char *scheme;
    char *user;
    char *password;
    char *host;
    char *port;
    char *path;
    char *params;
    char *query;
    char *fragment;
    char *buffer;
} URL_Parts;

/*
 * Function Declarations.
 */
void url_parts_initialize(URL_Parts *parts);
void url_parts_finalize(URL_Parts *parts);
const char *url_parts_url(URL_Parts *parts);
const char *url_parts_scheme(URL_Parts *parts);
const char *url_parts_user(URL_Parts *parts);
const char *url_parts_password(URL_Parts *parts);
const char *url_parts_host(URL_Parts *parts);
const char *url_parts_port(URL_Parts *parts);
const char *url_parts_path(URL_Parts *parts);
const char *url_parts_params(URL_Parts *parts);
const char *url_parts_query(URL_Parts *parts);
const char *url_parts_fragment(URL_Parts *parts);
int url_parts_parse(URL_Parts *, const char *parts);
void url_parts_print(URL_Parts *parts);

#endif /* not URLPARTS_H */
