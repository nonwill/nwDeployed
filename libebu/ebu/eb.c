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

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#ifdef ENABLE_EBNET
#include "ebnet.h"
#endif
#include "build-post.h"

/*
 * Initialize the library.
 */
EB_Error_Code
eb_initialize_library(void)
{
    EB_Error_Code error_code;

    eb_initialize_log();

    LOG(("in: eb_initialize_library()"));
    LOG(("aux: EB Library version %s", EB_VERSION_STRING));

    eb_initialize_default_hookset();
#ifdef ENABLE_NLS
    bindtextdomain(EB_TEXT_DOMAIN_NAME, EB_LOCALEDIR);
#endif
#ifdef ENABLE_EBNET
    ebnet_initialize();
#endif
    if (zio_initialize_library() < 0) {
	error_code = EB_ERR_MEMORY_EXHAUSTED;
	goto failed;
    }

    LOG(("out: eb_initialize_library() = %s", eb_error_string(EB_SUCCESS)));

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_initialize_library() = %s", eb_error_string(error_code)));
    return error_code;
}


/*
 * Finalize the library.
 */
void
eb_finalize_library(void)
{
    LOG(("in: eb_finalize_library()"));

    zio_finalize_library();
#ifdef ENABLE_EBNET
    ebnet_finalize();
#endif

    LOG(("out: eb_finalize_library()"));
}
