/*
 * Copyright 2026 The Concurrency Kit authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "pthread.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <errno.h>
#include <stdlib.h>

C_ASSERT(sizeof(pthread_mutex_t) == sizeof(SRWLOCK));

struct ck_pthread_context {
	HANDLE handle;
	void *(*start_routine)(void *);
	void *arg;
	void *result;
};

static DWORD __stdcall
thread_proc(void *context)
{
	struct ck_pthread_context *thread = context;
	thread->result = thread->start_routine(thread->arg);
	return 0;
}

int
pthread_create(pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg)
{
	pthread_t context;

	if (attr != NULL) {
		return EINVAL;
	}

	context = malloc(sizeof(*context));
	if (context == NULL) {
		return EAGAIN;
	}

	context->start_routine = start_routine;
	context->arg = arg;
	context->result = NULL;

	context->handle = CreateThread(NULL, 0, thread_proc, context, 0, NULL);
	if (context->handle == NULL) {
		free(context);
		return EAGAIN;
	}

	*thread = context;
	return 0;
}

int
pthread_join(pthread_t thread, void **value_ptr)
{
	WaitForSingleObject(thread->handle, INFINITE);
	if (value_ptr != NULL) {
		*value_ptr = thread->result;
	}
	CloseHandle(thread->handle);
	free(thread);
	return 0;
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	AcquireSRWLockExclusive((SRWLOCK *)mutex);
	return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{

	ReleaseSRWLockExclusive((SRWLOCK *)mutex);
	return 0;
}
