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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Character type tests and conversions.
 */
#define ASCII_ISDIGIT(c) ('0' <= (c) && (c) <= '9')
#define ASCII_ISUPPER(c) ('A' <= (c) && (c) <= 'Z')
#define ASCII_ISLOWER(c) ('a' <= (c) && (c) <= 'z')
#define ASCII_ISALPHA(c) \
 (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c) \
 (ASCII_ISUPPER(c) || ASCII_ISLOWER(c) || ASCII_ISDIGIT(c))
#define ASCII_ISXDIGIT(c) \
 (ASCII_ISDIGIT(c) || ('A' <= (c) && (c) <= 'F') || ('a' <= (c) && (c) <= 'f'))
#define ASCII_TOUPPER(c) (('a' <= (c) && (c) <= 'z') ? (c) - 0x20 : (c))
#define ASCII_TOLOWER(c) (('A' <= (c) && (c) <= 'Z') ? (c) + 0x20 : (c))

#include "urlparts.h"

/*
 * Unexported functions.
 */
static void url_parts_canonicalize_path(char *path);
static void url_parts_expand_hex(char *string);
static void url_parts_convert_to_lower(char *string);


/*
 * Initialize a query.
 */
void
url_parts_initialize(URL_Parts *parts)
{
    /*
     * Set all URL parts and whole URL to NULL.
     */
    parts->url      = NULL;
    parts->scheme   = NULL;
    parts->user     = NULL;
    parts->password = NULL;
    parts->host     = NULL;
    parts->port     = NULL;
    parts->path     = NULL;
    parts->params   = NULL;
    parts->query    = NULL;
    parts->fragment = NULL;

    parts->buffer   = NULL;
}


/*
 * Finalize a query.
 * All allocated memories in `queue' are also disposed.
 */
void
url_parts_finalize(URL_Parts *parts)
{
    /*
     * Set all URL parts and whole URL to NULL.
     */
    parts->scheme   = NULL;
    parts->user     = NULL;
    parts->password = NULL;
    parts->host     = NULL;
    parts->port     = NULL;
    parts->path     = NULL;
    parts->params   = NULL;
    parts->query    = NULL;
    parts->fragment = NULL;

    /*
     * Dispose memories assigned to `parts->url' and `parts->buffer'.
     */
    if (parts->url != NULL) {
	free(parts->url);
	parts->url = NULL;
    }

    if (parts->buffer != NULL) {
	free(parts->buffer);
	parts->buffer = NULL;
    }
}


/*
 * Return whole URL of a parsed URL.
 */
const char *
url_parts_url(URL_Parts *parts)
{
    return parts->url;
}


/*
 * Return scheme part of a parsed URL.
 */
const char *
url_parts_scheme(URL_Parts *parts)
{
    return parts->scheme;
}


/*
 * Return user part of a parsed URL.
 */
const char *
url_parts_user(URL_Parts *parts)
{
    return parts->user;
}


/*
 * Return password part of a parsed URL.
 */
const char *
url_parts_password(URL_Parts *parts)
{
    return parts->password;
}


/*
 * Return host part of a parsed URL.
 */
const char *
url_parts_host(URL_Parts *parts)
{
    return parts->host;
}


/*
 * Return port part of a parsed URL.
 */
const char *
url_parts_port(URL_Parts *parts)
{
    return parts->port;
}


/*
 * Return path part of a parsed URL.
 */
const char *
url_parts_path(URL_Parts *parts)
{
    return parts->path;
}


/*
 * Return params part of a parsed URL.
 */
const char *
url_parts_params(URL_Parts *parts)
{
    return parts->params;
}


/*
 * Return query part of a parsed URL.
 */
const char *
url_parts_query(URL_Parts *parts)
{
    return parts->query;
}


/*
 * Return fragment part of a parsed URL.
 */
const char *
url_parts_fragment(URL_Parts *parts)
{
    return parts->fragment;
}


/*
 * Parse an URL and fragment.
 *
 * `url' is an absolute or relative URL and optional fragment.
 * The function resolves the `url' into 9 parts; scheme, user, password,
 * host, port, path, params, query and fragment.
 *
 * [scheme ":"] ["//" [<user>][":" password] "@"]["host" [":" port]]
 *     ["/" path] [";" params] ["?" query] ["#" fragment]
 *
 * The result is put into `parts'.  This functions fails only when
 * memory is exhausted.  The function returns -1 in this case.
 * It returns 0, upon successful.
 */
int
url_parts_parse(URL_Parts *parts, const char *url)
{
    char *separator;
    char *url_p;
    size_t url_length;
    char *right_bracket;

    /*
     * Re-initialize if `url' has already been used.
     */
    if (parts->buffer != NULL) {
	url_parts_finalize(parts);
	url_parts_initialize(parts);
    }

    /*
     * Copy `url' to `parts->url' and `parts->buffer'.
     */
    url_length = strlen(url);
    parts->url = (char *)malloc(url_length + 1);
    if (parts->url == NULL)
	goto failed;
    memcpy(parts->url, url, url_length + 1);

    parts->buffer = (char *)malloc(url_length + 1);
    if (parts->buffer == NULL)
	goto failed;
    memcpy(parts->buffer, url, url_length + 1);

    /*
     * Get a fragment.
     */
    url_p = parts->buffer;
    separator = strchr(url_p, '#');
    if (separator != NULL) {
	if (*(separator + 1) != '\0')
	    parts->fragment = separator + 1;
	*separator = '\0';
    }

    /*
     * Get a scheme name.
     */
    separator = strchr(url_p, ':');
    if (separator != NULL) {
	char *p;

	for (p = url_p; *p != ':'; p++) {
	    if (!ASCII_ISALNUM(*p) && *p != '+' && *p != '.' && *p != '-')
		break;
	}
	if (*p == ':') {
	    parts->scheme = url_p;
	    *p = '\0';
	    url_p = p + 1;
	}
    }

    /*
     * Get a network location.
     */
    if (*url_p == '/' && *(url_p + 1) == '/') {
	char *netloc = NULL;
	char *hostport = NULL;
	char *userpass = NULL;
	char *p1, *p2;

	/*
	 * Shift left 2 characters to insert a terminator ('\0') for
	 * the network location, and a missed slash (`/').
	 *
	 * "//www.foo.co.jp/path..." --> "www.foo.co.jp\0\0/path..."
	 * "//www.foo.co.jp\0" --> "www.foo.co.jp\0/\0"
	 */
	netloc = url_p;
	for (p1 = url_p + 2, p2 = url_p;
	     *p1 != '/' && *p1 != '\0'; p1++, p2++)
	    *p2 = *p1;
	*p2++ = '\0';
	if (*p1 != '\0')
	    url_p = p1;
	else {
	    *p2 = '/';
	    url_p = p2;
	}

	/*
	 * Separate `netloc' into "<user>:<password>" and "<host>:<port>".
	 */
	separator = strchr(netloc, '@');
	if (separator != NULL) {
	    if (separator != netloc)
		userpass = netloc;
	    if (*(separator + 1) != '\0')
		hostport = separator + 1;
	    *separator = '\0';
	} else {
	    hostport = netloc;
	    userpass = NULL;
	}

	/*
	 * Get user and password.
	 */
	if (userpass != NULL) {
	    separator = strchr(userpass, ':');
	    if (separator != NULL) {
		if (separator != userpass)
		    parts->user = userpass;
		if (*(separator + 1) != '\0')
		    parts->password = separator + 1;
		*separator = '\0';
	    } else {
		parts->user = userpass;
	    }
	}

	/*
	 * Get host and port.
	 * IPv6 address is enclosed in `[' and `]'.
	 */
	if (hostport != NULL) {
		if (*hostport == '[') {
		    right_bracket = strchr(hostport + 1, ']');
		    if (right_bracket == NULL)
			separator = NULL;
		    else {
			if (*(right_bracket + 1) == ':'
			    || *(right_bracket + 1) == '\0') {
			    hostport++;
			    *right_bracket = '\0';
			}
			separator = strchr(right_bracket + 1, ':');
		    }
		} else {
		    separator = strchr(hostport, ':');
		}
	}

	if (separator != NULL) {
	    if (separator != hostport)
		parts->host = hostport;
	    if (*(separator + 1) != '\0')
		parts->port = separator + 1;
	    *separator = '\0';
	} else {
	    parts->host = hostport;
	}
    }

    /*
     * Parse query.
     */
    separator = strchr(url_p, '?');
    if (separator != NULL) {
	if (*(separator + 1) != '\0')
	    parts->query = separator + 1;
	*separator = '\0';
    }

    /*
     * Parse parameters.
     * The part is deleted if present.
     */
    separator = strchr(url_p, ';');
    if (separator != NULL) {
	if (*(separator + 1) != '\0')
	    parts->params = separator + 1;
	*separator = '\0';
    }

    /*
     * Parse path.
     */
    if (*url_p != '\0')
	parts->path = url_p;

    /*
     * Normalize parts.
     */
    if (parts->scheme != NULL) {
	url_parts_expand_hex(parts->scheme);
	url_parts_convert_to_lower(parts->scheme);
    }
    if (parts->user != NULL) {
	url_parts_expand_hex(parts->user);
    }
    if (parts->password != NULL) {
	url_parts_expand_hex(parts->password);
    }
    if (parts->host != NULL) {
	url_parts_expand_hex(parts->host);
	url_parts_convert_to_lower(parts->host);
    }
    if (parts->port != NULL) {
	url_parts_expand_hex(parts->port);
    }
    if (parts->path != NULL) {
	url_parts_expand_hex(parts->path);
	url_parts_canonicalize_path(parts->path);
    }
    if (parts->params != NULL) {
	url_parts_expand_hex(parts->params);
    }
    if (parts->query != NULL) {
	url_parts_expand_hex(parts->query);
    }
    if (parts->fragment != NULL) {
	url_parts_expand_hex(parts->fragment);
    }

    return 0;

    /*
     * An error occurs...
     */
  failed:
    url_parts_finalize(parts);
    return -1;
}


/*
 * Canonicalize the path in URL.
 *
 * Replace "." and ".." segments in the path.
 * All occurrences of "." segment are removed.
 * All occurrences of "<segment>/.." segments where <segment> is not
 * equal to "..", are removed.  This process is repeated until no
 * match pattern remains.
 *
 * For example:
 * 	"/A/./a.html"          --->  "/A/a.html"
 * 	"/A/../a.html"         --->  "/a.html"
 * 	"/A/B/C/../../a.html"  --->  "/A/a.html"
 * 	"/../a.html"           --->  "/../a.html" (*1)
 *
 * (*1) If a correspondig parent segment is not exist in the path,
 * we don't remove "/..".
 */
static void
url_parts_canonicalize_path(char *path)
{
    char *source = path;
    char *destination = path;
    char *unfixed_root = path;
    char *slash;

    while (*source != '\0') {
	if (*source != '/') {
	    *destination++ = *source++;
	    continue;
	}

	/*
	 * `*source' is slash (`/').
	 */
	if (*(source + 1) == '/') {
	    /*
	     * "//" -- Ignore 2nd slash ("/").
	     */
	    source++;
	} else if (*(source + 1) == '.'
	    && *(source + 2) == '/') {
	    /*
	     * "/./" -- Current segment itself.  Removed.
	     */
	    if (unfixed_root != destination) {
		source += 2;
	    } else {
		*destination++ = *source++;
		*destination++ = *source++;
		unfixed_root += 2;
	    }
	} else if (*(source + 1) == '.' && *(source + 2) == '.'
	    && *(source + 3) == '/') {
	    /*
	     * "/../" -- Back to the parent segment.
	     */
	    if (unfixed_root != destination) {
		source += 3;
		*destination = '\0';
		slash = strrchr(unfixed_root, '/');
		if (slash != NULL)
		    destination = slash;
		else
		    destination = path;
	    } else {
		*destination++ = *source++;
		*destination++ = *source++;
		*destination++ = *source++;
		unfixed_root += 3;
	    }
	} else {
	    /*
	     * "/" -- Single "/".
	     */
	    *destination++ = *source++;
	}
    }
    *destination = '\0';

    slash = strrchr(unfixed_root, '/');
    if (slash != NULL
	&& slash != unfixed_root
	&& *(slash + 1) == '.' && *(slash + 2) == '\0') {
	/*
	 * The path is end with "/.",  Remove ".".
	 * After the canonicalization, the path is end with "/".
	 */
	*(slash + 1) = '\0';
    } else if (slash != NULL
	&& slash != unfixed_root
	&& *(slash + 1) == '.' && *(slash + 2) == '.'
	&& *(slash + 3) == '\0') {
	/*
	 * The path is end with "/..",  Back to the parent segment.
	 * After the canonicalization, the path is end with "/".
	 */
	*slash = '\0';
	slash = strrchr(unfixed_root, '/');
	if (slash == NULL) {
	    *path = '/';
	    *(path + 1) = '\0';
	} else
	    *(slash + 1) = '\0';
    }
}

/*
 * All characters except for the followings are expandable characters.
 * Converting "%<HEX><HEX>" to that character doesn't affect URL parsing.
 *
 *	unsafe   ::= CTL | SP | <"> | "#" | "%" | "<" | ">"
 *	reserved ::= ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"
 *
 * `expandable_hex_table[]' is used to examine whether a character is
 * expandable or not.
 */
static const char expandable_hex_table[] = {
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, /* 0x00 ... 0x0f    */
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, /* 0x10 ... 0x1f    */
    0, 1, 0, 0,  1, 0, 0, 1,  1, 1, 1, 0,  1, 1, 1, 0, /*  !"#$%&'()*+,-./ */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0, /* 0123456789:;<=>? */
    0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* @ABCDEFGHIJKLMNO */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* PQRSTUVWXYZ[\]^_ */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* `abcdefghijklmno */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* pqrstuvwxyz{|}~  */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0x80 ... 0x8f    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0x90 ... 0x9f    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xa0 ... 0xaf    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xb0 ... 0xbf    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xc0 ... 0xcf    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xd0 ... 0xdf    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xe0 ... 0xef    */
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, /* 0xf0 ... 0xff    */
};

/*
 * Expand all "%<HEX><HEX>" in an URL part to original characters.
 */
static void
url_parts_expand_hex(char *string)
{
    char *source = string;
    char *destination = string;
    int hex1;
    int hex2;
    int c;

    while (*source != '\0') {
	/*
	 * "%<HEX><HEX>" occurs.
	 * Unescape it if the character is not unsafe or reserved.
	 */
	if (*source == '%'
	    && ASCII_ISXDIGIT(*(source + 1))
	    && ASCII_ISXDIGIT(*(source + 2))) {
	    hex1 = *(source + 1);
	    hex2 = *(source + 2);
	    c = 0;

	    if ('0' <= hex1 && hex1 <= '9')
		c += (hex1 - '0') << 4;
	    else if ('A' <= hex1 && hex1 <= 'F')
		c += (hex1 - 'A' + 0x0a) << 4;
	    else if ('a' <= hex1 && hex1 <= 'f')
		c += (hex1 - 'a' + 0x0a) << 4;

	    if ('0' <= hex2 && hex2 <= '9')
		c += (hex2 - '0');
	    else if ('A' <= hex2 && hex2 <= 'F')
		c += (hex2 - 'A' + 0x0a);
	    else if ('a' <= hex2 && hex2 <= 'f')
		c += (hex2 - 'a' + 0x0a);

	    if (expandable_hex_table[c])
		*(unsigned char *)destination++ = c;
	    else {
		*destination++ = '%';
		*destination++ = hex1;
		*destination++ = hex2;
	    }
	    source += 3;

	} else
	    *destination++ = *source++;
    }

    *destination = '\0';
}


/*
 * Convert upper case letters in a stinrg `string' to lower case.
 * This function is for the `scheme' and `host' parts in an URL,
 * since they are case insensitive,
 */
static void
url_parts_convert_to_lower(char *string)
{
    char *p;

    for (p = string; *p != '\0'; p++) {
	if (ASCII_ISUPPER(*p))
	    *p = 'a' + (*p - 'A');
    }
}


void
url_parts_print(URL_Parts *parts)
{
    printf("url parts = {\n");
    if (parts->scheme != NULL)
	printf("  scheme = %s\n", parts->scheme);
    if (parts->user != NULL)
	printf("  user = %s\n", parts->user);
    if (parts->password != NULL)
	printf("  password = %s\n", parts->password);
    if (parts->host != NULL)
	printf("  host = %s\n", parts->host);
    if (parts->port != NULL)
	printf("  port = %s\n", parts->port);
    if (parts->path != NULL)
	printf("  path = %s\n", parts->path);
    if (parts->params != NULL)
	printf("  params = %s\n", parts->params);
    if (parts->query != NULL)
	printf("  query = %s\n", parts->query);
    if (parts->fragment != NULL)
	printf("  fragment = %s\n", parts->fragment);
    printf("}\n");
    fflush(stdout);
}

/*
 * Main for test.
 */
#ifdef TEST

int
main(int argc, char *argv[])
{
    URL_Parts url;
    int i;

    if (argc < 2) {
	fprintf(stderr, "usage: %s URL\n", argv[0]);
	exit(1);
    }

    url_parts_initialize(&url);
    url_parts_parse(&url, argv[1]);
    url_parts_print(&url);
    fflush(stdout);
    url_parts_finalize(&url);
}

#endif
