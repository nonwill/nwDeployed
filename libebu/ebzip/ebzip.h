/*                                                            -*- C -*-
 * Copyright (c) 2001-2006  Motoyuki Kasahara
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

#ifndef EBZIP_H
#define EBZIP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>
#endif

#ifdef ENABLE_LIBDEFLATE
#include <libdeflate.h>
#else
#include <zlib.h>
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);
#endif

/*
 * O_BINARY flag for open().
 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

/*
 * stat macros.
 */
#ifndef S_ISREG
#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#endif

/*
 * The maximum length of path name.
 */
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else /* not MAXPATHLEN */
#define PATH_MAX	1024
#endif /* not MAXPATHLEN */
#endif /* not PATH_MAX */

#include "ebu/eb.h"
#include "ebu/error.h"
#include "ebu/font.h"
#include "ebu/build-post.h"

#include "getopt.h"
#include "getumask.h"
#include "makedir.h"
#include "samefile.h"
#include "strlist.h"
#include "yesno.h"

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
 * File name suffixes.
 */
#define EBZIP_SUFFIX_NONE		""
#define EBZIP_SUFFIX_EBZ		".ebz"
#define EBZIP_SUFFIX_ORG		".org"

/*
 * Defaults input and output directories.
 */
#define EBZIP_DEFAULT_BOOK_DIRECTORY	"."
#define EBZIP_DEFAULT_OUTPUT_DIRECTORY	"."

/*
 * Information output interval.
 */
#define EBZIP_PROGRESS_INTERVAL_FACTOR	1024

/*
 * Overwrite modes.
 */
#define EBZIP_OVERWRITE_CONFIRM		0
#define EBZIP_OVERWRITE_FORCE		1
#define EBZIP_OVERWRITE_NO		2

/*
 * Defaults.
 */
#define EBZIP_DEFAULT_LEVEL		0
#define EBZIP_DEFAULT_KEEP		0
#define EBZIP_DEFAULT_QUIET		0
#define EBZIP_DEFAULT_TEST		0
#define EBZIP_DEFAULT_OVERWRITE		EBZIP_OVERWRITE_CONFIRM

#define EBZIP_DEFAULT_SKIP_FONT		0
#define EBZIP_DEFAULT_SKIP_GRAPHIC	0
#define EBZIP_DEFAULT_SKIP_MOVIE	0
#define EBZIP_DEFAULT_SKIP_SOUND	0
#define EBZIP_DEFAULT_SLICE_NUMBER	64
#define EBZIP_MAX_SLICE_NUMBER		10000

/*
 * Region in HONMON or START file which ebzip doesn't compress.
 */
#define EBZIP_MAX_SPEEDUP_REGION_COUNT	3

typedef struct {
    int start_page;
    int end_page;
} Zip_Speedup_Region;

typedef struct {
    int region_count;
    Zip_Speedup_Region regions[EBZIP_MAX_SPEEDUP_REGION_COUNT];
} Zip_Speedup;


/*
 * Global variables.
 */
extern const char *program_name;
extern const char *program_version;
extern const char *invoked_name;

extern int ebzip_level;
extern int ebzip_keep_flag;
extern int ebzip_quiet_flag;
extern int ebzip_test_flag;
extern int ebzip_overwrite_mode;

extern int ebzip_skip_flag_font;
extern int ebzip_skip_flag_graphic;
extern int ebzip_skip_flag_movie;
extern int ebzip_skip_flag_sound;

extern int ebzip_slice_number;

extern String_List unlinking_files;

/*
 * Function declarations.
 */
/* copyfile.c */
int ebzip_copy_file(const char *out_file_name, const char *in_file_name);
int ebzip_copy_files_in_directory(const char *out_directory_name,
    const char *in_directory_name);

/* ebzip1.c */
int ebzip1_slice(char *out_buffer, size_t *out_byte_length, char *in_buffer,
    size_t in_byte_length);

/* sebxa.c */
int rewrite_sebxa_start(const char *file_name, int index_page);
int get_sebxa_indexes(const char *file_name, int index_page,
    off_t *index_location, off_t *index_base, off_t *zio_start_location,
    off_t *zio_end_location);

/* unzipbook.c */
int ebzip_unzip_book(const char *out_top_path, const char *book_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count);

/* unzipfile.c */
int ebzip_unzip_file(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code);
int ebzip_unzip_start_file(const char *out_file_name,
    const char *in_file_name, Zio_Code in_zio_code, int index_page);

/* zipbook.c */
int ebzip_zip_book(const char *out_top_path, const char *book_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count);

/* zipfile.c */
int ebzip_zip_file(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code, Zip_Speedup *speedup);
int ebzip_zip_start_file(const char *out_file_name, const char *in_file_name,
    Zio_Code in_zio_code, int index_page, Zip_Speedup *speedup);

/* zipinfobook.c */
int ebzip_zipinfo_book(const char *book_path,
    char subbook_name_list[][EB_MAX_DIRECTORY_NAME_LENGTH + 1],
    int subbook_name_count);

/* zipinfofile.c */
int ebzip_zipinfo_file(const char *in_file_name, Zio_Code in_zio_code);
int ebzip_zipinfo_start_file(const char *in_file_name, Zio_Code in_zio_code,
    int index_page);

/* sppedup.c */
void ebzip_initialize_zip_speedup(Zip_Speedup *speedup);
void ebzip_finalize_zip_speedup(Zip_Speedup *speedup);
int ebzip_set_zip_speedup(Zip_Speedup *speedup, const char *file_name,
    Zio_Code zio_code, int index_page);
int ebzip_is_speedup_slice(Zip_Speedup *speedup, int slice, int zip_level);

/* unlinkfile.c */
int unlink_files_add(const char *file_name);
void unlink_files();

#endif /* EBZIP_H */
