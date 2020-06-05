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
#include <sys/stat.h>

/*
 * If the files `file_name1' and `file_name2' are identical (i.e. the are
 * written on a same device and a same i-node is assigned to them), 1 is
 * returned.  Otherwise, 0 is returned.
 */
int
is_same_file(const char *file_name1, const char *file_name2)
{
    struct stat st1, st2;

    if (stat(file_name1, &st1) != 0 || stat(file_name2, &st2) != 0)
	return 0;

#ifndef DOS_FILE_PATH
    if (st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino)
	return 0;

#else /* DOS_FILE_PATH */
    /* Can't rely on st_ino and st_dev, use other fields: */
    if (st1.st_mode != st2.st_mode
	|| st1.st_uid != st2.st_uid
	|| st1.st_gid != st2.st_gid
	|| st1.st_size != st2.st_size
	|| st1.st_atime != st2.st_atime
	|| st1.st_mtime != st2.st_mtime
	|| st1.st_ctime != st2.st_ctime)
	return 0;
#endif /* DOS_FILE_PATH */

    return 1;
}
