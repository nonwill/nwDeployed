/*
 * Copyright (c) 1997-2006  Motoyuki Kasahara
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

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#ifdef ENABLE_EBNET
#include "ebnet.h"
#endif
#include "build-post.h"

#if defined(DOS_FILE_PATH) && defined(HAVE_MBSTRING_H)
/* a path may contain double-byte chars in SJIS. */
#include <mbstring.h>
#define strchr	_mbschr
#define strrchr	_mbsrchr
#endif


#ifndef DOS_FILE_PATH

/*
 * Canonicalize `path_name' (UNIX version).
 * Convert a path name to an absolute path.
 */
EB_Error_Code
eb_canonicalize_path_name(char *path_name)
{
    char cwd[EB_MAX_PATH_LENGTH + 1];
    char temporary_path_name[EB_MAX_PATH_LENGTH + 1];
    char *last_slash;

    if (*path_name != '/') {
	/*
	 * `path_name' is an relative path.  Convert to an absolute
	 * path.
	 */
	if (getcwd(cwd, EB_MAX_PATH_LENGTH + 1) == NULL)
	    return EB_ERR_FAIL_GETCWD;
	if (EB_MAX_PATH_LENGTH < strlen(cwd) + 1 + strlen(path_name))
	    return EB_ERR_TOO_LONG_FILE_NAME;

	if (strcmp(path_name, ".") == 0) {
	    strcpy(path_name, cwd);
	} else if (strncmp(path_name, "./", 2) == 0) {
	    sprintf(temporary_path_name, "%s/%s", cwd, path_name + 2);
	    strcpy(path_name, temporary_path_name);
	} else {
	    sprintf(temporary_path_name, "%s/%s", cwd, path_name);
	    strcpy(path_name, temporary_path_name);
	}
    }

    /*
     * Unless `path_name' is "/", eliminate `/' in the tail of the
     * path name.
     */
    last_slash = strrchr(path_name, '/');
    if (last_slash != path_name && *(last_slash + 1) == '\0')
	*last_slash = '\0';

    return EB_SUCCESS;
}

#else /* DOS_FILE_PATH */

/*
 * Canonicalize `path_name' (DOS version).
 * Convert a path name to an absolute path with drive letter unless
 * that is an UNC path.
 *
 * Original version by KSK Jan/30/1998.
 * Current version by Motoyuki Kasahara.
 */
EB_Error_Code
eb_canonicalize_path_name(char *path_name)
{
    char cwd[EB_MAX_PATH_LENGTH + 1];
    char temporary_path_name[EB_MAX_PATH_LENGTH + 1];
    char *slash;
    char *last_backslash;

    /*
     * Replace `/' with `\\'.
     */
    slash = path_name;
    for (;;) {
	slash = strchr(slash, '/');
	if (slash == NULL)
	    break;
	*slash++ = '\\';
    }

    if (*path_name == '\\' && *(path_name + 1) == '\\') {
	/*
	 * `path_name' is UNC path.  Nothing to be done.
	 */
    } else if (ASCII_ISALPHA(*path_name) && *(path_name + 1) == ':') {
	/*
	 * `path_name' has a drive letter.
	 * Nothing to be done if it is an absolute path.
	 */
	if (*(path_name + 2) != '\\') {
	    /*
	     * `path_name' is a relative path.
	     * Covert the path name to an absolute path.
	     */
#ifdef __MINGW32__
        if (getcwd(cwd, EB_MAX_PATH_LENGTH + 1) == NULL) {
#elif _MSC_VER
        if (_getcwd(ASCII_TOUPPER(*path_name) - 'A' + 1, cwd,
                    EB_MAX_PATH_LENGTH + 1) == NULL) {
#else
	    if (getdcwd(ASCII_TOUPPER(*path_name) - 'A' + 1, cwd,
		EB_MAX_PATH_LENGTH + 1) == NULL) {
#endif
		return EB_ERR_FAIL_GETCWD;
	    }
	    if (EB_MAX_PATH_LENGTH < strlen(cwd) + 1 + strlen(path_name + 2))
		return EB_ERR_TOO_LONG_FILE_NAME;
	    sprintf(temporary_path_name, "%s\\%s", cwd, path_name + 2);
	    strcpy(path_name, temporary_path_name);
	}
    } else if (*path_name == '\\') {
	/*
	 * `path_name' has no drive letter and is an absolute path.
	 * Add a drive letter to the path name.
	 */
	if (getcwd(cwd, EB_MAX_PATH_LENGTH + 1) == NULL)
	    return EB_ERR_FAIL_GETCWD;
	cwd[1] = '\0';
	if (EB_MAX_PATH_LENGTH < strlen(cwd) + 1 + strlen(path_name))
	    return EB_ERR_TOO_LONG_FILE_NAME;
	sprintf(temporary_path_name, "%s:%s", cwd, path_name);
	strcpy(path_name, temporary_path_name);

    } else {
	/*
	 * `path_name' has no drive letter and is a relative path.
	 * Add a drive letter and convert it to an absolute path.
	 */
	if (getcwd(cwd, EB_MAX_PATH_LENGTH + 1) == NULL)
	    return EB_ERR_FAIL_GETCWD;

	if (EB_MAX_PATH_LENGTH < strlen(cwd) + 1 + strlen(path_name))
	    return EB_ERR_TOO_LONG_FILE_NAME;
	sprintf(temporary_path_name, "%s\\%s", cwd, path_name);
	strcpy(path_name, temporary_path_name);
    }


    /*
     * Now `path_name' is `X:\...' or `\\...'.
     * Unless it is "X:\", eliminate `\' in the tail of the path name.
     */
    last_backslash = strrchr(path_name, '\\');
    if (ASCII_ISALPHA(*path_name)) {
	if (last_backslash != path_name + 2 && *(last_backslash + 1) == '\0')
	    *last_backslash = '\0';
    } else {
	if (last_backslash != path_name + 1 && *(last_backslash + 1) == '\0')
	    *last_backslash = '\0';
    }

    return EB_SUCCESS;
}

#endif /* DOS_FILE_PATH */


/*
 * Canonicalize file name.
 *    - Suffix including dot is removed
 *    - Version including semicolon is removed.
 *    - Letters are converted to upper case.
 *
 * We minght fail to load a file after we fix the file name.
 * If loading the file is tried again, we need the original file name,
 * not fixed file name.  Therefore, we get orignal file name from fixed
 * file name using this function.
 */
void
eb_canonicalize_file_name(char *file_name)
{
    char *p;

    for (p = file_name; *p != '\0' && *p != '.' && *p != ';'; p++)
	*p = ASCII_TOUPPER(*p);
    *p = '\0';
}


/*
 * Rewrite `directory_name' to a real directory name in the `path' directory.
 *
 * If a directory matched to `directory_name' exists, then EB_SUCCESS is
 * returned, and `directory_name' is rewritten to that name.  Otherwise
 * EB_ERR_BAD_DIR_NAME is returned.
 */
EB_Error_Code
eb_fix_directory_name(const char *path, char *directory_name)
{
    struct dirent *entry;
    DIR *dir;

#ifdef ENABLE_EBNET
    if (is_ebnet_url(path))
	return ebnet_fix_directory_name(path, directory_name);
#endif

    /*
     * Open the directory `path'.
     */
    dir = opendir(path);
    if (dir == NULL)
        goto failed;

    for (;;) {
        /*
         * Read the directory entry.
         */
        entry = readdir(dir);
        if (entry == NULL)
            goto failed;

	if (strcasecmp(entry->d_name, directory_name) == 0)
	    break;
    }

    strcpy(directory_name, entry->d_name);
    closedir(dir);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (dir != NULL)
	closedir(dir);
    return EB_ERR_BAD_DIR_NAME;
}


/*
 * Rewrite `sub_directory_name' to a real sub directory name in the
 * `path/directory_name' directory.
 *
 * If a directory matched to `sub_directory_name' exists, then EB_SUCCESS
 * is returned, and `directory_name' is rewritten to that name.  Otherwise
 * EB_ERR_BAD_FILE_NAME is returned.
 */
EB_Error_Code
eb_fix_directory_name2(const char *path, const char *directory_name,
    char *sub_directory_name)
{
    char sub_path_name[EB_MAX_PATH_LENGTH + 1];

    eb_compose_path_name(path, directory_name, sub_path_name);
    return eb_fix_directory_name(sub_path_name, sub_directory_name);
}


/*
 * Fix suffix of `path_name'.
 *
 * If `suffix' is an empty string, delete suffix from `path_name'.
 * Otherwise, add `suffix' to `path_name'.
 */
void
eb_fix_path_name_suffix(char *path_name, const char *suffix)
{
    char *base_name;
    char *dot;
    char *semicolon;

#ifndef DOS_FILE_PATH
    base_name = strrchr(path_name, '/');
#else
    if (is_ebnet_url(path_name))
	base_name = strrchr(path_name, '/');
    else
	base_name = strrchr(path_name, '\\');
#endif
    if (base_name == NULL)
	base_name = path_name;
    else
	base_name++;

    dot = strchr(base_name, '.');
    semicolon = strchr(base_name, ';');

    if (*suffix == '\0') {
	/*
	 * Remove `.xxx' from `fixed_file_name':
	 *   foo.xxx    -->  foo
	 *   foo.xxx;1  -->  foo;1
	 *   foo.       -->  foo.     (unchanged)
	 *   foo.;1     -->  foo.;1   (unchanged)
	 */
	if (dot != NULL && *(dot + 1) != '\0' && *(dot + 1) != ';') {
	    if (semicolon != NULL)
		sprintf(dot, ";%c", *(semicolon + 1));
	    else
		*dot = '\0';
	}
    } else {
	/*
	 * Add `.xxx' to `fixed_file_name':
	 *   foo       -->  foo.xxx
	 *   foo.      -->  foo.xxx
	 *   foo;1     -->  foo.xxx;1
	 *   foo.;1    -->  foo.xxx;1
	 *   foo.xxx   -->  foo.xxx    (unchanged)
	 */
	if (dot != NULL) {
	    if (semicolon != NULL)
		sprintf(dot, "%s;%c", suffix, *(semicolon + 1));
	    else
		strcpy(dot, suffix);
	} else {
	    if (semicolon != NULL)
		sprintf(semicolon, "%s;%c", suffix, *(semicolon + 1));
	    else
		strcat(base_name, suffix);
	}
    }
}


#define FOUND_NONE		0
#define FOUND_EBZ		1
#define FOUND_BASENAME		2
#define FOUND_ORG		3

/*
 * Rewrite `found_file_name' to a real file name in the `path_name'
 * directory.
 *
 * If a file matched to `target_file_name' exists, then EB_SUCCESS
 * is returned, and `found_file_name' is rewritten to that name.
 * Otherwise EB_ERR_BAD_FILE_NAME is returned.
 *
 * Note that `target_file_name' must not contain `.' or excceed
 * EB_MAX_DIRECTORY_NAME_LENGTH characters.
 */
EB_Error_Code
eb_find_file_name(const char *path_name, const char *target_file_name,
    char *found_file_name)
{
    char ebz_target_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char org_target_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char candidate_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    DIR *dir;
    struct dirent *entry;
    size_t d_namlen;
    int found = FOUND_NONE;

#ifdef ENABLE_EBNET
    if (is_ebnet_url(path_name)) {
	return ebnet_find_file_name(path_name, target_file_name,
	    found_file_name);
    }
#endif

    strcpy(ebz_target_file_name, target_file_name);
    strcat(ebz_target_file_name, ".ebz");
    strcpy(org_target_file_name, target_file_name);
    strcat(org_target_file_name, ".org");
    candidate_file_name[0] = '\0';

    /*
     * Open the directory `path_name'.
     */
    dir = opendir(path_name);
    if (dir == NULL)
	goto failed;

    for (;;) {
	/*
	 * Read the directory entry.
	 */
	entry = readdir(dir);
	if (entry == NULL)
	    break;

	/*
	 * Compare the given file names and the current entry name.
	 * We consider they are matched when one of the followings
	 * is true:
	 *
	 *   <target name>          == <entry name>
	 *   <target name>+";1'     == <entry name>
	 *   <target name>+"."      == <entry name>
	 *   <target name>+".;1"    == <entry name>
	 *   <target name>+".ebz"   == <entry name>
	 *   <target name>+".ebz;1" == <entry name>
	 *   <target name>+".org"   == <entry name>
	 *   <target name>+".org;1" == <entry name>
	 *
	 * All the comparisons are done without case sensitivity.
	 * We support version number ";1" only.
	 */
	d_namlen = strlen(entry->d_name);
	if (2 < d_namlen
	    && *(entry->d_name + d_namlen - 2) == ';'
	    && ASCII_ISDIGIT(*(entry->d_name + d_namlen - 1))) {
	    d_namlen -= 2;
	}
	if (1 < d_namlen && *(entry->d_name + d_namlen - 1) == '.')
	    d_namlen--;

	if (strcasecmp(entry->d_name, ebz_target_file_name) == 0
	    && *(ebz_target_file_name + d_namlen) == '\0'
	    && found < FOUND_EBZ) {
	    strcpy(candidate_file_name, entry->d_name);
	    found = FOUND_EBZ;
	}
	if (strncasecmp(entry->d_name, target_file_name, d_namlen) == 0
	    && *(target_file_name + d_namlen) == '\0'
	    && found < FOUND_BASENAME) {
	    strcpy(candidate_file_name, entry->d_name);
	    found = FOUND_BASENAME;
	}
	if (strcasecmp(entry->d_name, org_target_file_name) == 0
	    && *(org_target_file_name + d_namlen) == '\0'
	    && found < FOUND_ORG) {
	    strcpy(candidate_file_name, entry->d_name);
	    found = FOUND_ORG;
	    break;
	}
    }

    if (found == FOUND_NONE)
	goto failed;

    closedir(dir);
    strcpy(found_file_name, candidate_file_name);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    if (dir != NULL)
	closedir(dir);
    return EB_ERR_BAD_FILE_NAME;
}


/*
 * Rewrite `file_name' to a real file name in the directory
 * `path_name/sub_directory_name'.
 *
 * If a file matched to `file_name' exists, then EB_SUCCESS is returned,
 * and `file_name' is rewritten to that name.  Otherwise EB_ERR_BAD_FILE_NAME
 * is returned.
 */
EB_Error_Code
eb_find_file_name2(const char *path_name, const char *sub_directory_name,
    const char *target_file_name, char *found_file_name)
{
    char sub_path_name[EB_MAX_PATH_LENGTH + 1];

    eb_compose_path_name(path_name, sub_directory_name, sub_path_name);
    return eb_find_file_name(sub_path_name, target_file_name, found_file_name);
}


EB_Error_Code
eb_find_file_name3(const char *path_name, const char *sub_directory_name,
    const char *sub2_directory_name, const char *target_file_name,
    char *found_file_name)
{
    char sub2_path_name[EB_MAX_PATH_LENGTH + 1];

    eb_compose_path_name2(path_name, sub_directory_name, sub2_directory_name,
	sub2_path_name);
    return eb_find_file_name(sub2_path_name, target_file_name,
	found_file_name);
}


/*
 * Compose a file name
 *     `path_name/file_name'
 * and copy it into `composed_path_name'.
 */
void
eb_compose_path_name(const char *path_name, const char *file_name,
    char *composed_path_name)
{
#ifndef DOS_FILE_PATH
    if (strcmp(path_name, "/") == 0)
	sprintf(composed_path_name, "%s%s", path_name, file_name);
    else
	sprintf(composed_path_name, "%s/%s", path_name, file_name);
#else
    if (is_ebnet_url(path_name))
	sprintf(composed_path_name, "%s/%s", path_name, file_name);
    else if (ASCII_ISALPHA(*path_name) && strcmp(path_name + 1, ":\\") == 0)
	sprintf(composed_path_name, "%s%s", path_name, file_name);
    else
	sprintf(composed_path_name, "%s\\%s", path_name, file_name);
#endif
}


/*
 * Compose a file name
 *     `path_name/sub_directory/file_name'
 * and copy it into `composed_path_name'.
 */
void
eb_compose_path_name2(const char *path_name, const char *sub_directory_name,
    const char *file_name, char *composed_path_name)
{
#ifndef DOS_FILE_PATH
    if (strcmp(path_name, "/") == 0) {
	sprintf(composed_path_name, "%s%s/%s",
	    path_name, sub_directory_name, file_name);
    } else {
	sprintf(composed_path_name, "%s/%s/%s",
	    path_name, sub_directory_name, file_name);
    }
#else
    if (is_ebnet_url(path_name)) {
	sprintf(composed_path_name, "%s/%s/%s",
	    path_name, sub_directory_name, file_name);
    } else if (ASCII_ISALPHA(*path_name)
	&& strcmp(path_name + 1, ":\\") == 0) {
	sprintf(composed_path_name, "%s%s\\%s",
	    path_name, sub_directory_name, file_name);
    } else {
	sprintf(composed_path_name, "%s\\%s\\%s",
	    path_name, sub_directory_name, file_name);
    }
#endif
}


/*
 * Compose a file name
 *     `path_name/sub_directory/sub2_directory/file_name'
 * and copy it into `composed_path_name'.
 */
void
eb_compose_path_name3(const char *path_name, const char *sub_directory_name,
    const char *sub2_directory_name, const char *file_name,
    char *composed_path_name)
{
#ifndef DOS_FILE_PATH
    if (strcmp(path_name, "/") == 0) {
	sprintf(composed_path_name, "%s%s/%s/%s",
	    path_name, sub_directory_name, sub2_directory_name, file_name);
    } else {
	sprintf(composed_path_name, "%s/%s/%s/%s",
	    path_name, sub_directory_name, sub2_directory_name, file_name);
    }
#else
    if (is_ebnet_url(path_name)) {
	sprintf(composed_path_name, "%s/%s/%s/%s",
	    path_name, sub_directory_name, sub2_directory_name, file_name);
    } else if (ASCII_ISALPHA(*path_name)
	&& strcmp(path_name + 1, ":\\") == 0) {
	sprintf(composed_path_name, "%s%s\\%s\\%s",
	    path_name, sub_directory_name, sub2_directory_name, file_name);
    } else {
	sprintf(composed_path_name, "%s\\%s\\%s\\%s",
	    path_name, sub_directory_name, sub2_directory_name, file_name);
    }
#endif
}


/*
 * Compose movie file name from argv[], and copy the result to
 * `composed_file_name'.  Note that upper letters are converted to lower
 * letters.
 *
 * If a file `composed_path_name' exists, then EB_SUCCESS is returned.
 * Otherwise EB_ERR_BAD_FILE_NAME is returned.
 */
EB_Error_Code
eb_compose_movie_file_name(const unsigned int *argv, char *composed_file_name)
{
    unsigned short jis_characters[EB_MAX_DIRECTORY_NAME_LENGTH];
    const unsigned int *arg_p;
    char *composed_p;
    unsigned short c;
    int i;

    /*
     * Initialize `jis_characters[]'.
     */
    for (i = 0, arg_p = argv;
	 i + 1 < EB_MAX_DIRECTORY_NAME_LENGTH; i += 2, arg_p++) {
	jis_characters[i]     = (*arg_p >> 16) & 0xffff;
	jis_characters[i + 1] = (*arg_p)       & 0xffff;
    }
    if (i < EB_MAX_DIRECTORY_NAME_LENGTH)
	jis_characters[i]     = (*arg_p >> 16) & 0xffff;

    /*
     * Compose file name.
     */
    for (i = 0, composed_p = composed_file_name;
	 i < EB_MAX_DIRECTORY_NAME_LENGTH; i++, composed_p++) {
	c = jis_characters[i];
	if (c == 0x2121 || c == 0x0000)
	    break;
	if ((0x2330 <= c && c <= 0x2339) || (0x2361 <= c && c <= 0x237a))
	    *composed_p = c & 0xff;
	else if (0x2341 <= c && c <= 0x235a)
	    *composed_p = (c | 0x20) & 0xff;
	else
	    return EB_ERR_BAD_FILE_NAME;
    }

    *composed_p = '\0';

    return EB_SUCCESS;
}


/*
 * This function is similar to eb_compose_movie_file_name(), but it
 * returns full path of the movie file name.
 */
EB_Error_Code
eb_compose_movie_path_name(EB_Book *book, const unsigned int *argv,
    char *composed_path_name)
{
    EB_Subbook *subbook;
    EB_Error_Code error_code;
    char composed_file_name[EB_MAX_FILE_NAME_LENGTH + 1];

    /*
     * Lock the book.
     */
    eb_lock(&book->lock);
    LOG(("in: eb_compose_movie_path_name(book=%d, argv=%x)",
	(int)book->code, argv));

    /*
     * Current subbook must have been set.
     */
    if (book->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_SUB;
	goto failed;
    }
    subbook = book->subbook_current;

    error_code = eb_compose_movie_file_name(argv, composed_file_name);
    if (error_code != EB_SUCCESS)
	goto failed;

    error_code = eb_find_file_name3(book->path, subbook->directory_name,
	subbook->movie_directory_name, composed_file_name, composed_file_name);
    if (error_code != EB_SUCCESS)
	goto failed;
    eb_compose_path_name3(book->path, subbook->directory_name,
	subbook->movie_directory_name, composed_file_name, composed_path_name);

    LOG(("out: eb_compse_movie_path_name() = %s",
	eb_error_string(EB_SUCCESS)));

    eb_unlock(&book->lock);
    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_compse_movie_path_name() = %s",
	eb_error_string(error_code)));
    eb_unlock(&book->lock);
    return error_code;
}


/*
 * Decompose movie file name into argv[].  This is the reverse of
 * eb_compose_movie_file_name().  Note that lower letters are converted
 * to upper letters.
 *
 * EB_SUCCESS is returned upon success, EB_ERR_BAD_FILE_NAME otherwise.
 */
EB_Error_Code
eb_decompose_movie_file_name(unsigned int *argv,
    const char *composed_file_name)
{
    unsigned short jis_characters[EB_MAX_DIRECTORY_NAME_LENGTH];
    unsigned int *arg_p;
    const char *composed_p;
    int i;

    /*
     * Initialize `jis_characters[]'.
     */
    for (i = 0; i < EB_MAX_DIRECTORY_NAME_LENGTH; i++)
	jis_characters[i] = 0x0000;

    /*
     * Set jis_characters[].
     */
    for (i = 0, composed_p = composed_file_name;
	 i < EB_MAX_DIRECTORY_NAME_LENGTH && *composed_p != '\0';
	 i++, composed_p++) {
	if ('0' <= *composed_p && *composed_p <= '9')
	    jis_characters[i] = 0x2330 + (*composed_p - '0');
	else if ('A' <= *composed_p && *composed_p <= 'Z')
	    jis_characters[i] = 0x2341 + (*composed_p - 'A');
	else if ('a' <= *composed_p && *composed_p <= 'z')
	    jis_characters[i] = 0x2341 + (*composed_p - 'a');
	else
	    return EB_ERR_BAD_FILE_NAME;
    }
    if (*composed_p != '\0')
	return EB_ERR_BAD_FILE_NAME;

    /*
     * Decompose file name.
     */
    for (i = 0, arg_p = argv;
	 i + 1 < EB_MAX_DIRECTORY_NAME_LENGTH; i += 2, arg_p++) {
	*arg_p = (jis_characters[i] << 16) | jis_characters[i + 1];
    }
    if (i < EB_MAX_DIRECTORY_NAME_LENGTH) {
	*arg_p++ = jis_characters[i] << 16;
    }
    *arg_p = '\0';

    return EB_SUCCESS;
}


void
eb_path_name_zio_code(const char *path_name, Zio_Code default_zio_code,
    Zio_Code *zio_code)
{
    const char *base_name;
    const char *dot;

#ifndef DOS_FILE_PATH
    base_name = strrchr(path_name, '/');
#else
    if (is_ebnet_url(path_name))
	base_name = strrchr(path_name, '/');
    else
	base_name = strrchr(path_name, '\\');
#endif
    if (base_name != NULL)
	base_name++;
    else
	base_name = path_name;

    dot = strchr(base_name, '.');
    if (dot != NULL && strncasecmp(dot, ".ebz", 4) == 0)
	*zio_code = ZIO_EBZIP1;
    else if (dot != NULL && strncasecmp(dot, ".org", 4) == 0)
	*zio_code = ZIO_PLAIN;
    else
	*zio_code = default_zio_code;
}
