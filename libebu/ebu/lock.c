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
#include "defs.h"
#include "build-post.h"

/*
 * Examine whether built library supports Pthread.
 */
int
eb_pthread_enabled(void)
{
#ifdef ENABLE_PTHREAD
    return 1;
#else
    return 0;
#endif
}


/*
 * These functions are compiled only when ENABLE_PTHREAD is defined.
 */
#ifdef ENABLE_PTHREAD

/*
 * Ininialize a lock manager.
 */
void
eb_initialize_lock(EB_Lock *lock)
{
    pthread_mutex_init(&lock->lock_count_mutex, NULL);
    pthread_mutex_init(&lock->entity_mutex, NULL);
    lock->lock_count = 0;
}


/*
 * Finalize a lock manager.
 */
void
eb_finalize_lock(EB_Lock *lock)
{
    /* Nothing to be done. */
}


/*
 * Lock an entity.
 */
void
eb_lock(EB_Lock *lock)
{
    pthread_mutex_lock(&lock->lock_count_mutex);
    if (lock->lock_count == 0)
	pthread_mutex_lock(&lock->entity_mutex);
    lock->lock_count++;
    pthread_mutex_unlock(&lock->lock_count_mutex);
}


/*
 * Unlock an entity.
 */
void
eb_unlock(EB_Lock *lock)
{
    pthread_mutex_lock(&lock->lock_count_mutex);
    if (0 < lock->lock_count) {
	lock->lock_count--;
	if (lock->lock_count == 0)
	    pthread_mutex_unlock(&lock->entity_mutex);
    }
    pthread_mutex_unlock(&lock->lock_count_mutex);
}

#endif /* ENABLE_PTHREAD */
