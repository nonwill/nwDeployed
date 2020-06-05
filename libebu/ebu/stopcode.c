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
#include "appendix.h"
#include "text.h"
#include "build-post.h"

/*
 * Examine whether the current subbook in `appendix' has a stop-code.
 */
int
eb_have_stop_code(EB_Appendix *appendix)
{
    eb_lock(&appendix->lock);
    LOG(("in: eb_have_stop_code(appendix=%d)", (int)appendix->code));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL)
	goto failed;

    if (appendix->subbook_current->stop_code0 == 0)
	goto failed;

    LOG(("out: eb_have_stop_code() = %d", 1));
    eb_unlock(&appendix->lock);

    return 1;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_have_stop_code() = %d", 0));
    eb_unlock(&appendix->lock);
    return 0;
}


/*
 * Return the stop-code of the current subbook in `appendix'.
 */
EB_Error_Code
eb_stop_code(EB_Appendix *appendix, int *stop_code)
{
    EB_Error_Code error_code;

    eb_lock(&appendix->lock);
    LOG(("in: eb_stop_code(appendix=%d)", (int)appendix->code));

    /*
     * Current subbook must have been set.
     */
    if (appendix->subbook_current == NULL) {
	error_code = EB_ERR_NO_CUR_APPSUB;
	goto failed;
    }

    if (appendix->subbook_current->stop_code0 == 0) {
	error_code = EB_ERR_NO_STOPCODE;
	goto failed;
    }

    stop_code[0] = appendix->subbook_current->stop_code0;
    stop_code[1] = appendix->subbook_current->stop_code1;

    LOG(("out: eb_stop_code(stop_code=%d,%d) = %s",
	stop_code[0], stop_code[1], eb_error_string(EB_SUCCESS)));
    eb_unlock(&appendix->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    stop_code[0] = -1;
    stop_code[1] = -1;
    LOG(("out: eb_stop_code() = %s", eb_error_string(error_code)));
    eb_unlock(&appendix->lock);
    return error_code;
}


