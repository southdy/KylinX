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

#if defined(__clang_analyzer__) && !defined(SDL_DISABLE_ANALYZE_MACROS)
#define SDL_DISABLE_ANALYZE_MACROS 1
#endif

#include "../SDL_internal.h"

/* This file contains portable memory management functions for SDL */
#include "SDL_stdinc.h"
#include "SDL_atomic.h"
#include "SDL_error.h"

/* Memory functions used by SDL that can be replaced by the application */
static struct
{
    SDL_malloc_func malloc_func;
    SDL_calloc_func calloc_func;
    SDL_realloc_func realloc_func;
    SDL_free_func free_func;
    SDL_atomic_t num_allocations;
} s_mem = {
    malloc, calloc, realloc, free, { 0 }
};

void SDL_GetMemoryFunctions(SDL_malloc_func *malloc_func,
                            SDL_calloc_func *calloc_func,
                            SDL_realloc_func *realloc_func,
                            SDL_free_func *free_func)
{
    if (malloc_func) {
        *malloc_func = s_mem.malloc_func;
    }
    if (calloc_func) {
        *calloc_func = s_mem.calloc_func;
    }
    if (realloc_func) {
        *realloc_func = s_mem.realloc_func;
    }
    if (free_func) {
        *free_func = s_mem.free_func;
    }
}

int SDL_SetMemoryFunctions(SDL_malloc_func malloc_func,
                           SDL_calloc_func calloc_func,
                           SDL_realloc_func realloc_func,
                           SDL_free_func free_func)
{
    if (!malloc_func) {
        return SDL_InvalidParamError("malloc_func");
    }
    if (!calloc_func) {
        return SDL_InvalidParamError("calloc_func");
    }
    if (!realloc_func) {
        return SDL_InvalidParamError("realloc_func");
    }
    if (!free_func) {
        return SDL_InvalidParamError("free_func");
    }

    s_mem.malloc_func = malloc_func;
    s_mem.calloc_func = calloc_func;
    s_mem.realloc_func = realloc_func;
    s_mem.free_func = free_func;
    return 0;
}

int SDL_GetNumAllocations(void)
{
    return SDL_AtomicGet(&s_mem.num_allocations);
}

void *SDL_malloc(size_t size)
{
    void *mem;

    if (!size) {
        size = 1;
    }

    mem = s_mem.malloc_func(size);
    if (mem) {
        SDL_AtomicIncRef(&s_mem.num_allocations);
    }
    return mem;
}

void *SDL_calloc(size_t nmemb, size_t size)
{
    void *mem;

    if (!nmemb || !size) {
        nmemb = 1;
        size = 1;
    }

    mem = s_mem.calloc_func(nmemb, size);
    if (mem) {
        SDL_AtomicIncRef(&s_mem.num_allocations);
    }
    return mem;
}

void *SDL_realloc(void *ptr, size_t size)
{
    void *mem;

    if (!ptr && !size) {
        size = 1;
    }

    mem = s_mem.realloc_func(ptr, size);
    if (mem && !ptr) {
        SDL_AtomicIncRef(&s_mem.num_allocations);
    }
    return mem;
}

void SDL_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    s_mem.free_func(ptr);
    (void)SDL_AtomicDecRef(&s_mem.num_allocations);
}

/* vi: set ts=4 sw=4 expandtab: */
