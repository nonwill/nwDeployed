/*
 * Copyright (c) 2003-2006  Motoyuki Kasahara
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
#ifdef ENABLE_WINSOCK2
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#include "build-pre.h"
#include "eb.h"
#include "build-post.h"
#include "dummyin6.h"

#if !defined(HAVE_GETADDRINFO) || !defined(HAVE_GETNAMEINFO)
#include "getaddrinfo.h"
#endif

#include "ebnet.h"

#ifndef IF_NAMESIZE
#ifdef IFNAMSIZ
#define IF_NAMESIZE             IFNAMSIZ
#else
#define IF_NAMESIZE             16
#endif
#endif

#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST		0
#endif
#ifndef NI_WITHSCOPEID
#define NI_WITHSCOPEID		0
#endif

#ifdef ENABLE_WINSOCK2
#define close closesocket
#endif

/*
 * Maximum length (including NUL) of a TCP/UDP port number string.
 */
#define IN_PORTSTRLEN		6

/*
 * Socket entry pool.
 */
typedef struct EBNet_Socket_Entry_Struct EBNet_Socket_Entry;
struct EBNet_Socket_Entry_Struct {
    /* IPv6 or IPv4 Address of peer */
    char address[INET6_ADDRSTRLEN + IF_NAMESIZE];

    /* destination port */
    in_port_t port;

    /* socket file */
    int file;

    /* reference count of the entry */
    int reference_count;

    /* object ID of the entry */
    int reference_id;

    /* lost synchronization flag */
    int lost_sync;

    /* next and previous entries. */
    EBNet_Socket_Entry *next;
    EBNet_Socket_Entry *back;

    /* book name */
    char book_name[EBNET_MAX_BOOK_NAME_LENGTH + 1];

    /* file path */
    char file_path[EB_MAX_RELATIVE_PATH_LENGTH + 1];

    /* current file pointer */
    off_t offset;

    /* file size */
    off_t file_size;
};

static EBNet_Socket_Entry *ebnet_socket_entries;
static EBNet_Socket_Entry *ebnet_socket_entry_cache;

/*
 * Host name information cache.
 */
typedef struct {
    /* hostname or IP address */
    char *host;

    /* IPv6 address of `host' */
    char ipv6_address[INET6_ADDRSTRLEN + IF_NAMESIZE];

    /* IPv4 address of `host' */
    char ipv4_address[INET_ADDRSTRLEN];
} EBNet_Host_Info;

static EBNet_Host_Info ebnet_host_info_cache;


/*
 * Hello and Bye hooks.
 *
 * The multiplex module invokes the hello hook each time it establish
 * a new connection with a server, and invokes the bye hook each time
 * it closes a last socket connected with a server.
 */
static int (*hello_hook)(int file);
static int (*bye_hook)(int file);

/*
 * Unexported functions.
 */
static void ebnet_get_addresses(const char *host, char *ipv6_address,
    char *ipv4_address);
static EBNet_Socket_Entry *ebnet_find_multiplex_entry(const char *ipv6_address,
    const char *ipv4_address, int port, int family);
static int ebnet_create_new_connection(const char *address, int port);
static void ebnet_add_socket_entry(EBNet_Socket_Entry *new_entry);
static void ebnet_delete_socket_entry(EBNet_Socket_Entry *target_entry);
static EBNet_Socket_Entry *ebnet_find_socket_entry(int file);


/*
 * Initialize the multiplex module.
 */
void
ebnet_initialize_multiplex(void)
{
    ebnet_socket_entries = NULL;
    ebnet_socket_entry_cache = NULL;

    ebnet_host_info_cache.host = NULL;
    ebnet_host_info_cache.ipv6_address[0] = '\0';
    ebnet_host_info_cache.ipv4_address[0] = '\0';

    hello_hook = NULL;
    bye_hook = NULL;
}


/*
 * Finalize the multiplex module.
 */
void
ebnet_finalize(void)
{
    while (ebnet_socket_entries != NULL) {
	close(ebnet_socket_entries->file);
	ebnet_delete_socket_entry(ebnet_socket_entries);
    }

    ebnet_socket_entries = NULL;
    ebnet_socket_entry_cache = NULL;

    if (ebnet_host_info_cache.host != NULL)
	free(ebnet_host_info_cache.host);
}


/*
 * Set hello hook.
 */
void
ebnet_set_hello_hook(int (*hook)(int file))
{
    hello_hook = hook;
}


/*
 * Set bye hook.
 */
void
ebnet_set_bye_hook(int (*hook)(int file))
{
    bye_hook = hook;
}


/*
 * Create a socket connected with a server.
 *
 * `host' is a host name or an IP address of the server.  `port' is
 * destination port number of the TCP connection.  `family' is protocol
 * family: PF_INET, PF_INET6 or PF_UNSPEC.
 *
 * Upon success, file descriptor of the socket is returned.
 * Otherwise -1 is returned.
 *
 * If there has been a socket entry in `ebnet_socket_entries' which is
 * connected with the server, this function simply duplicates the socket.
 */
int
ebnet_connect_socket(const char *host, int port, int family)
{
    char ipv6_address[INET6_ADDRSTRLEN + IF_NAMESIZE];
    char ipv4_address[INET_ADDRSTRLEN];
    EBNet_Socket_Entry *multiplex_entry = NULL;
    EBNet_Socket_Entry *new_entry = NULL;
    int new_file = -1;

    /*
     * Get IP addresses of `host'.
     */
    *ipv6_address = '\0';
    *ipv4_address = '\0';
    ebnet_get_addresses(host, ipv6_address, ipv4_address);

    switch (family) {
    case PF_UNSPEC:
	if (*ipv6_address == '\0' && *ipv4_address == '\0')
	    goto failed;
	break;

    case PF_INET6:
	if (*ipv6_address == '\0')
	    goto failed;
	break;

    case PF_INET:
	if (*ipv4_address == '\0')
	    goto failed;
	break;
    }

    /*
     * Search `ebnet_socket_entries' for a connection entry with
     * the server.
     */
    multiplex_entry = ebnet_find_multiplex_entry(ipv6_address, ipv4_address,
	port, family);

    /*
     * Create a socket entry.
     */
    new_entry = (EBNet_Socket_Entry *)malloc(sizeof(EBNet_Socket_Entry));
    if (new_entry == NULL)
	goto failed;

    new_entry->address[0]      = '\0';
    new_entry->port            = port;
    new_entry->file            = -1;
    new_entry->reference_count = 1;
    new_entry->reference_id    = -1;
    new_entry->lost_sync       = 0;
    new_entry->next            = NULL;
    new_entry->back            = NULL;
    new_entry->book_name[0]    = '\0';
    new_entry->file_path[0]    = '\0';
    new_entry->offset          = 0;
    new_entry->file_size       = 0;

    if (multiplex_entry != NULL) {
	/*
	 * There is an IPv6 or IPv4 socket with the server.
	 * Duplicate the socket entry.
	 */
#ifndef ENABLE_WINSOCK2
	new_file = dup(multiplex_entry->file);
#else /* ENABLE_WINSOCK2 */
	{
	    WSAPROTOCOL_INFO info;

	    if (WSADuplicateSocket(multiplex_entry->file,
		GetCurrentProcessId(), &info) != 0)
		goto failed;
	    new_file = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
		FROM_PROTOCOL_INFO, &info, 0, 0);
	}
#endif /* ENABLE_WINSOCK2 */
	if (new_file < 0)
	    goto failed;

	strcpy(new_entry->address, multiplex_entry->address);
	new_entry->file = new_file;
	new_entry->reference_count = multiplex_entry->reference_count;
	new_entry->reference_id = multiplex_entry->reference_id;

    } else {
	/*
	 * There is no socket connected with the server.
	 * Establish a connection with the server.
	 */
	do {
	    if ((family == PF_INET6 || family == PF_UNSPEC)
		&& *ipv6_address != '\0') {
		new_file = ebnet_create_new_connection(ipv6_address, port);
		if (0 <= new_file) {
		    strcpy(new_entry->address, ipv6_address);
		    new_entry->file = new_file;
		    new_entry->reference_id = new_file;
		    break;
		}
	    }
	    if ((family == PF_INET || family == PF_UNSPEC)
		&& *ipv4_address != '\0') {
		new_file = ebnet_create_new_connection(ipv4_address, port);
		if (0 <= new_file) {
		    strcpy(new_entry->address, ipv4_address);
		    new_entry->file = new_file;
		    new_entry->reference_id = new_file;
		    break;
		}
	    }
	    goto failed;

	} while (0);

    }

    /*
     * Add the entry to `ebnet_socket_entries'.
     */
    ebnet_add_socket_entry(new_entry);

    /*
     * Say hello.
     */
    if (multiplex_entry == NULL && hello_hook != NULL) {
	if (hello_hook(new_file) < 0)
	    goto failed;
    }

    return new_file;

    /*
     * An error occurs...
     */
  failed:
    if (new_entry != NULL) {
	if (ebnet_find_socket_entry(new_file) != NULL)
	    ebnet_delete_socket_entry(new_entry);
	else
	    free(new_entry);
    }
    if (new_file >= 0)
	close(new_file);
    return -1;
}


/*
 * Get IPv6 and IPv4 addresses of `host'.
 *
 * `host' may be either an host name or an IP address.
 * If `host' has an IPv6 address, the address is written on `ipv6_address'.
 * Otherwise an empty string is written.
 * If `host' has an IPv4 address, the address is written on `ipv4_address'.
 * Otherwise an empty string is written.
 */
static void
ebnet_get_addresses(const char *host, char *ipv6_address, char *ipv4_address)
{
    struct addrinfo hints;
    struct addrinfo *info_list = NULL;
    struct addrinfo *info;
    char dummy_service[IN_PORTSTRLEN];
    int gai_error;

    *ipv6_address = '\0';
    *ipv4_address = '\0';

    /*
     * Look up `ebnet_host_info_cache'.
     */
    if (ebnet_host_info_cache.host != NULL
	&& strcmp(ebnet_host_info_cache.host, host) == 0) {
	strcpy(ipv6_address, ebnet_host_info_cache.ipv6_address);
	strcpy(ipv4_address, ebnet_host_info_cache.ipv4_address);
	return;
    }

    /*
     * Get IP addresses using getaddrinfo().
     */
    hints.ai_flags = 0;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    gai_error = getaddrinfo(host, EBNET_DEFAULT_PORT, &hints, &info_list);
    if (gai_error != 0)
	return;

    for (info = info_list; info != NULL; info = info->ai_next) {
	if (info->ai_family != PF_INET6)
	    continue;
	gai_error = getnameinfo(info->ai_addr, info->ai_addrlen,
	    ipv6_address, INET6_ADDRSTRLEN + IF_NAMESIZE,
	    dummy_service, sizeof(dummy_service),
	    NI_NUMERICHOST | NI_NUMERICSERV | NI_WITHSCOPEID);
#if NI_WITHSCOPEID != 0
	if (gai_error != 0) {
	    gai_error = getnameinfo(info->ai_addr, info->ai_addrlen,
		ipv6_address, INET6_ADDRSTRLEN + IF_NAMESIZE,
		dummy_service, sizeof(dummy_service),
		NI_NUMERICHOST | NI_NUMERICSERV);
	}
#endif
	if (gai_error == 0)
	    break;
	*ipv6_address = '\0';
    }

    for (info = info_list; info != NULL; info = info->ai_next) {
	if (info->ai_family != PF_INET)
	    continue;
	gai_error = getnameinfo(info->ai_addr, info->ai_addrlen,
	    ipv4_address, INET_ADDRSTRLEN,
	    dummy_service, sizeof(dummy_service),
	    NI_NUMERICHOST | NI_NUMERICSERV);
	if (gai_error == 0)
	    break;
	*ipv4_address = '\0';
    }

    freeaddrinfo(info_list);

    /*
     * Update `ebnet_host_info_cache'.
     */
    if (ebnet_host_info_cache.host != NULL) {
	free(ebnet_host_info_cache.host);
	ebnet_host_info_cache.host = NULL;
    }
    ebnet_host_info_cache.host = malloc(strlen(host) + 1);
    if (ebnet_host_info_cache.host != NULL) {
	strcpy(ebnet_host_info_cache.host, host);
	strcpy(ebnet_host_info_cache.ipv6_address, ipv6_address);
	strcpy(ebnet_host_info_cache.ipv4_address, ipv4_address);
    }

    return;
}


/*
 * Find `ebnet_socket_entries' for a socket entry which matches with
 * port, family and either `ipv6_address' or `ipv4_address'.
 *
 * If found, the function returns the entry.  Otherwise it returns NULL.
 */
static EBNet_Socket_Entry *
ebnet_find_multiplex_entry(const char *ipv6_address, const char *ipv4_address,
    int port, int family)
{
    EBNet_Socket_Entry *entry;

    /*
     * Inspect `ebnet_socket_entry_cache'.
     */
    if (ebnet_socket_entry_cache != NULL
	&& (family == PF_INET6 || family == PF_UNSPEC)
	&& strcasecmp(ebnet_socket_entry_cache->address, ipv6_address) == 0
	&& ebnet_socket_entry_cache->port == port
	&& !ebnet_socket_entry_cache->lost_sync) {
	return ebnet_socket_entry_cache;
    }
    if (ebnet_socket_entry_cache != NULL
	&& (family == PF_INET || family == PF_UNSPEC)
	&& strcasecmp(ebnet_socket_entry_cache->address, ipv4_address) == 0
	&& ebnet_socket_entry_cache->port == port
	&& !ebnet_socket_entry_cache->lost_sync) {
	return ebnet_socket_entry_cache;
    }

    /*
     * Then inspect entires in `ebnet_socket_entries'.
     */
    for (entry = ebnet_socket_entries; entry != NULL; entry = entry->next) {
	if ((family == PF_INET6 || family == PF_UNSPEC)
	    && strcasecmp(entry->address, ipv6_address) == 0
	    && entry->port == port
	    && !entry->lost_sync) {
	    ebnet_socket_entry_cache = entry;
	    return entry;
	}
	if ((family == PF_INET || family == PF_UNSPEC)
	    && strcasecmp(entry->address, ipv4_address) == 0
	    && entry->port == port
	    && !entry->lost_sync) {
	    ebnet_socket_entry_cache = entry;
	    return entry;
	}
    }

    return NULL;
}


/*
 * Establish a TCP connection with an EBNET server.
 *
 * `host' is a host name or an IP address of the server.  `port' is
 * destination port number of the TCP connection.
 *
 * Upon success, file descriptor of the socket is returned.
 * Otherwise -1 is returned.
 */
static int
ebnet_create_new_connection(const char *address, int port)
{
    struct addrinfo hints;
    struct addrinfo *info_list = NULL;
    int new_file = -1;
    int gai_error;
    char port_string[IN_PORTSTRLEN];
#ifdef O_NONBLOCK
    int file_flags;
#endif

    if (port < 0 || 65535 < port)
	goto failed;
    sprintf(port_string, "%d", port);

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    gai_error = getaddrinfo(address, port_string, &hints, &info_list);
    if (gai_error != 0)
	goto failed;

    new_file = socket(info_list->ai_addr->sa_family, SOCK_STREAM, 0);
    if (new_file < 0)
	goto failed;
    if (connect(new_file, info_list->ai_addr, info_list->ai_addrlen) < 0)
	goto failed;

#ifdef O_NONBLOCK
    file_flags = fcntl(new_file, F_GETFL, 0);
    if (file_flags >= 0)
	fcntl(new_file, F_SETFL, file_flags | O_NONBLOCK);
#endif

    freeaddrinfo(info_list);

    return new_file;

    /*
     * An error occurs...
     */
  failed:
    if (info_list != NULL)
	freeaddrinfo(info_list);
    if (new_file >= 0)
	close(new_file);
    return -1;
}


/*
 * Add `new_entry' to `ebnet_socket_entries'.
 */
static void
ebnet_add_socket_entry(EBNet_Socket_Entry *new_entry)
{
    EBNet_Socket_Entry *entry;
    int reference_count;

    /*
     * Increment reference counts.
     */
    reference_count = 1;
    for (entry = ebnet_socket_entries; entry != NULL; entry = entry->next) {
	if (entry->reference_id == new_entry->reference_id) {
	    entry->reference_count++;
	    reference_count = entry->reference_count;
	}
    }

    new_entry->reference_count = reference_count;

    /*
     * Add `new_entry' to `ebnet_socket_entries'.
     */
    if (ebnet_socket_entries != NULL)
	ebnet_socket_entries->back = new_entry;
    new_entry->back = NULL;
    new_entry->next = ebnet_socket_entries;
    ebnet_socket_entries = new_entry;

    ebnet_socket_entry_cache = new_entry;
}


/*
 * Delete `target_entry' from `ebnet_socket_entries'.
 */
static void
ebnet_delete_socket_entry(EBNet_Socket_Entry *target_entry)
{
    EBNet_Socket_Entry *entry;
    int new_reference_id;

    if (ebnet_socket_entry_cache == target_entry)
	ebnet_socket_entry_cache = NULL;

    /*
     * Delete `target_entry' from `ebnet_socket_entries'.
     */
    if (target_entry->next != NULL)
	target_entry->next->back = target_entry->back;
    if (target_entry->back != NULL)
	target_entry->back->next = target_entry->next;
    if (target_entry == ebnet_socket_entries)
	ebnet_socket_entries = ebnet_socket_entries->next;

    /*
     * Decrement reference counts and update reference ID.
     */
    entry = ebnet_socket_entries;
    while (entry != NULL) {
	if (entry->reference_id == target_entry->reference_id) {
	    new_reference_id = entry->file;
	    break;
	}
	entry = entry->next;
    }
    while (entry != NULL) {
	if (entry->reference_id == target_entry->reference_id) {
	    entry->reference_id = new_reference_id;
	    entry->reference_count--;
	}
	entry = entry->next;
    }

    /*
     * Dispose `target_entry'.
     */
    free(target_entry);
}


/*
 * Disconnect with a server.
 */
void
ebnet_disconnect_socket(int file)
{
    EBNet_Socket_Entry *entry;

    /*
     * Search `ebnet_socket_entries' for a connection entry with `file'.
     */
    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return;

    /*
     * Say good bye and close the connection.
     */
    if (entry->reference_count == 1 && !entry->lost_sync && bye_hook != NULL)
	bye_hook(entry->file);

    close(entry->file);
    ebnet_delete_socket_entry(entry);
}


/*
 * Reconnect `file'.
 * It returns `file' upon success, -1 otherwise.
 */
int
ebnet_reconnect_socket(int file)
{
    EBNet_Socket_Entry *old_entry;
    EBNet_Socket_Entry *new_entry = NULL;
    int new_file;

    /*
     * Search `ebnet_socket_entries' for a connection entry with `file'.
     */
    old_entry = ebnet_find_socket_entry(file);
    if (old_entry == NULL)
	goto failed;

    /*
     * Say good bye.
     */
    if (old_entry->reference_count == 1 && !old_entry->lost_sync
	&& bye_hook != NULL)
	bye_hook(old_entry->file);

    /*
     * Establish a new connection with the server.
     */
    ebnet_set_lost_sync(file);
    new_file = ebnet_connect_socket(old_entry->address, old_entry->port,
	PF_UNSPEC);
    if (new_file < 0)
	goto failed;

    new_entry = ebnet_find_socket_entry(new_file);
    if (new_entry == NULL)
	goto failed;

    /*
     * Replace `old_entry' by `new_entry'.
     */
    strcpy(new_entry->book_name, old_entry->book_name);
    strcpy(new_entry->file_path, old_entry->file_path);
    new_entry->offset = old_entry->offset;
    new_entry->file_size = old_entry->file_size;

    ebnet_delete_socket_entry(old_entry);

#ifndef ENABLE_WINSOCK2
    if (dup2(new_entry->file, file) < 0)
	goto failed;
    close(new_entry->file);

    if (new_entry->reference_id == new_entry->file)
	new_entry->reference_id = file;
    new_entry->file = file;
#else
    close(file);
#endif

    return new_entry->file;

    /*
     * An error occurs...
     */
  failed:
    if (new_entry != NULL && new_entry->file != file)
	ebnet_disconnect_socket(new_entry->file);
    return -1;
}


/*
 * Mark files as lost-synchronization.
 *
 * Socket entries in `ebnet_socket_entries' which has the same reference
 * ID as `file' are marked as lost-synchronization.
 * The function doesn't close the files, but ebnet_connect_socket()
 * doesn't duplicate lost-synchronized files.
 */
int
ebnet_set_lost_sync(int file)
{
    EBNet_Socket_Entry *entry;
    int reference_id;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;
    reference_id = entry->reference_id;

    for (entry = ebnet_socket_entries; entry != NULL; entry = entry->next) {
	if (entry->reference_id == reference_id)
	    entry->lost_sync = 1;
    }

    return 0;
}


/*
 * Search `ebnet_socket_entries' for a connection entry with `file'.
 */
static EBNet_Socket_Entry *
ebnet_find_socket_entry(int file)
{
    EBNet_Socket_Entry *entry;

    if (ebnet_socket_entry_cache != NULL
	&& ebnet_socket_entry_cache->file == file)
	return ebnet_socket_entry_cache;

    for (entry = ebnet_socket_entries; entry != NULL; entry = entry->next) {
	if (entry->file == file) {
	    ebnet_socket_entry_cache = entry;
	    return entry;
	}
    }

    return NULL;
}


/*
 * Set book name associated with `file'.
 */
int
ebnet_set_book_name(int file, const char *book_name)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    strncpy(entry->book_name, book_name, EBNET_MAX_BOOK_NAME_LENGTH + 1);
    *(entry->book_name + EBNET_MAX_BOOK_NAME_LENGTH) = '\0';

    return 0;
}


/*
 * Get book name associated with `file'.
 */
const char *
ebnet_get_book_name(int file)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return NULL;

    return entry->book_name;
}


/*
 * Set book name associated with `file'.
 */
int
ebnet_set_file_path(int file, const char *file_path)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    strncpy(entry->file_path, file_path, EB_MAX_RELATIVE_PATH_LENGTH + 1);
    *(entry->file_path + EB_MAX_RELATIVE_PATH_LENGTH + 1) = '\0';

    return 0;
}


/*
 * Get file path associated with `file'.
 */
const char *
ebnet_get_file_path(int file)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return NULL;

    return entry->file_path;
}


/*
 * Set file offset.
 */
int
ebnet_set_offset(int file, off_t offset)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    entry->offset = offset;
    return 0;
}


/*
 * Get file offset.
 */
off_t
ebnet_get_offset(int file)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    return entry->offset;
}


/*
 * Set file size.
 */
int
ebnet_set_file_size(int file, off_t file_size)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    entry->file_size = file_size;
    return 0;
}


/*
 * Get file size.
 */
off_t
ebnet_get_file_size(int file)
{
    EBNet_Socket_Entry *entry;

    entry = ebnet_find_socket_entry(file);
    if (entry == NULL)
	return -1;

    return entry->file_size;
}


