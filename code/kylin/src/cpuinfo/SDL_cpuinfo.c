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
#ifdef TEST_MAIN
#include "SDL_config.h"
#else
#include "../SDL_internal.h"
#endif

/* CPU feature detection for SDL */

#include "SDL_cpuinfo.h"
#include "SDL_assert.h"

size_t
SDL_SIMDGetAlignment(void)
{
    return sizeof(void *);  /* a good safe base value */
}

void *
SDL_SIMDAlloc(const size_t len)
{
    const size_t alignment = SDL_SIMDGetAlignment();
    const size_t padding = alignment - (len % alignment);
    const size_t padded = (padding != alignment) ? (len + padding) : len;
    Uint8 *retval = NULL;
    Uint8 *ptr = (Uint8 *) SDL_malloc(padded + alignment + sizeof (void *));
    if (ptr) {
        /* store the actual malloc pointer right before our aligned pointer. */
        retval = ptr + sizeof (void *);
        retval += alignment - (((size_t) retval) % alignment);
        *(((void **) retval) - 1) = ptr;
    }
    return retval;
}

void
SDL_SIMDFree(void *ptr)
{
    if (ptr) {
        void **realptr = (void **) ptr;
        realptr--;
        SDL_free(*(((void **) ptr) - 1));
    }
}


#ifdef TEST_MAIN

#include <stdio.h>

int
main()
{
    printf("CPU count: %d\n", SDL_GetCPUCount());
    printf("CPU type: %s\n", SDL_GetCPUType());
    printf("CPU name: %s\n", SDL_GetCPUName());
    printf("CacheLine size: %d\n", SDL_GetCPUCacheLineSize());
    printf("RAM: %d MB\n", SDL_GetSystemRAM());
    return 0;
}

#endif /* TEST_MAIN */

/* vi: set ts=4 sw=4 expandtab: */
