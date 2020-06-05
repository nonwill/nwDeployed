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

#ifndef EB_BUILD_PRE_H
#define EB_BUILD_PRE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <dirent.h>
#endif
#include <fcntl.h>
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#ifdef _MSC_VER
#include "dirent.h"
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#ifdef HAVE__GETDCWD
#define getcwd _getcwd
#define getdcwd _getdcwd
#endif

#ifdef ENABLE_PTHREAD
#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

/*
 * Mutual exclusion lock of Pthreads.
 */
#ifndef ENABLE_PTHREAD
#define pthread_mutex_lock(m)
#define pthread_mutex_unlock(m)
#endif

/*
 * stat() macros.
 */
#ifndef S_ISREG
#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#endif

/*
 * Flags for open().
 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

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

/*
 * Tricks for gettext.
 */
#ifdef ENABLE_NLS
#define _(string) gettext(string)
#ifdef gettext_noop
#define N_(string) gettext_noop(string)
#else
#define N_(string) (string)
#endif
#else
#define _(string) (string)
#define N_(string) (string)
#endif

/*
 * Fake missing function names.
 */
#ifndef HAVE_STRCASECMP
#define strcasecmp eb_strcasecmp
#define strncasecmp eb_strncasecmp
#endif

#ifndef HAVE_GETADDRINFO
#define addrinfo ebnet_addrinfo
#define getaddrinfo ebnet_getaddrinfo
#define freeaddrinfo ebnet_freeaddrinfo
#endif

#ifndef HAVE_GETNAMEINFO
#define getnameinfo ebnet_getnameinfo
#endif

#ifndef HAVE_GAI_STRERROR
#define gai_strerror ebnet_gai_strerror
#endif

#ifndef IN6ADDR_ANY_DECLARED
#define in6addr_any ebnet_in6addr_any
#endif

#ifndef IN6ADDR_LOOPBACK_DECLARED
#define in6addr_loopback ebnet_in6addr_loopback
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#ifdef _WIN64
  typedef __int64 ssize_t;
#else
  typedef int ssize_t;
#endif
#endif

#endif /* EB_BUILD_PRE_H */
