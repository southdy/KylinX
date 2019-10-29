/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../SDL_internal.h"

#include "SDL_atomic.h"

#include <stdatomic.h>

void
SDL_AtomicLock(SDL_SpinLock *lock)
{
    while (__sync_lock_test_and_set(lock, 1));
}

void
SDL_AtomicUnlock(SDL_SpinLock *lock)
{
    __sync_lock_release(lock);
}

SDL_bool
SDL_AtomicCAS(SDL_atomic_t *a, int oldval, int newval)
{
    return (SDL_bool) __sync_bool_compare_and_swap(&a->value, oldval, newval);
}

SDL_bool
SDL_AtomicCASPtr(void **a, void *oldval, void *newval)
{
    return __sync_bool_compare_and_swap(a, oldval, newval);
}

int
SDL_AtomicSet(SDL_atomic_t *a, int v)
{
    return __sync_lock_test_and_set(&a->value, v);
}

void*
SDL_AtomicSetPtr(void **a, void *v)
{
    return __sync_lock_test_and_set(a, v);
}

int
SDL_AtomicAdd(SDL_atomic_t *a, int v)
{
    return __sync_fetch_and_add(&a->value, v);
}

int
SDL_AtomicGet(SDL_atomic_t *a)
{
    return __atomic_load_n(&a->value, __ATOMIC_SEQ_CST);
}

void *
SDL_AtomicGetPtr(void **a)
{
    return __atomic_load_n(a, __ATOMIC_SEQ_CST);
}

/* vi: set ts=4 sw=4 expandtab: */
