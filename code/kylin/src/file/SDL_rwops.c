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

#include <stdio.h>
#include <limits.h>

/* This file provides a general interface for SDL to read and write
   data sources.  It can easily be extended to files, memory, etc.
*/

#include "SDL_endian.h"
#include "SDL_rwops.h"

#ifdef __APPLE__
#include "cocoa/SDL_rwopsbundlesupport.h"
#endif /* __APPLE__ */

#ifdef __ANDROID__
#include "../core/android/SDL_android.h"
#include "SDL_system.h"
#endif

#define FSEEK_OFF_MIN LONG_MIN
#define FSEEK_OFF_MAX LONG_MAX
#define fseek_off_t long

/* Functions to read/write stdio file pointers */

static Sint64 SDLCALL
stdio_size(SDL_RWops * context)
{
    Sint64 pos, size;

    pos = SDL_RWseek(context, 0, RW_SEEK_CUR);
    if (pos < 0) {
        return -1;
    }
    size = SDL_RWseek(context, 0, RW_SEEK_END);

    SDL_RWseek(context, pos, RW_SEEK_SET);
    return size;
}

static Sint64 SDLCALL
stdio_seek(SDL_RWops * context, Sint64 offset, int whence)
{
    int stdiowhence;

    switch (whence) {
    case RW_SEEK_SET:
        stdiowhence = SEEK_SET;
        break;
    case RW_SEEK_CUR:
        stdiowhence = SEEK_CUR;
        break;
    case RW_SEEK_END:
        stdiowhence = SEEK_END;
        break;
    default:
        return SDL_SetError("Unknown value for 'whence'");
    }

    if (offset < (Sint64)(FSEEK_OFF_MIN) || offset > (Sint64)(FSEEK_OFF_MAX)) {
        return SDL_SetError("Seek offset out of range");
    }

    if (fseek(context->hidden.stdio.fp, (fseek_off_t)offset, stdiowhence) == 0) {
        Sint64 pos = ftell(context->hidden.stdio.fp);
        if (pos < 0) {
            return SDL_SetError("Couldn't get stream offset");
        }
        return pos;
    }
    return SDL_Error(SDL_EFSEEK);
}

static size_t SDLCALL
stdio_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
    size_t nread;

    nread = fread(ptr, size, maxnum, context->hidden.stdio.fp);
    if (nread == 0 && ferror(context->hidden.stdio.fp)) {
        SDL_Error(SDL_EFREAD);
    }
    return nread;
}

static size_t SDLCALL
stdio_write(SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    size_t nwrote;

    nwrote = fwrite(ptr, size, num, context->hidden.stdio.fp);
    if (nwrote == 0 && ferror(context->hidden.stdio.fp)) {
        SDL_Error(SDL_EFWRITE);
    }
    return nwrote;
}

static int SDLCALL
stdio_close(SDL_RWops * context)
{
    int status = 0;
    if (context) {
        if (context->hidden.stdio.autoclose) {
            /* WARNING:  Check the return value here! */
            if (fclose(context->hidden.stdio.fp) != 0) {
                status = SDL_Error(SDL_EFWRITE);
            }
        }
        SDL_FreeRW(context);
    }
    return status;
}

/* Functions to read/write memory pointers */

static Sint64 SDLCALL
mem_size(SDL_RWops * context)
{
    return (Sint64)(context->hidden.mem.stop - context->hidden.mem.base);
}

static Sint64 SDLCALL
mem_seek(SDL_RWops * context, Sint64 offset, int whence)
{
    Uint8 *newpos;

    switch (whence) {
    case RW_SEEK_SET:
        newpos = context->hidden.mem.base + offset;
        break;
    case RW_SEEK_CUR:
        newpos = context->hidden.mem.here + offset;
        break;
    case RW_SEEK_END:
        newpos = context->hidden.mem.stop + offset;
        break;
    default:
        return SDL_SetError("Unknown value for 'whence'");
    }
    if (newpos < context->hidden.mem.base) {
        newpos = context->hidden.mem.base;
    }
    if (newpos > context->hidden.mem.stop) {
        newpos = context->hidden.mem.stop;
    }
    context->hidden.mem.here = newpos;
    return (Sint64)(context->hidden.mem.here - context->hidden.mem.base);
}

static size_t SDLCALL
mem_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
    size_t total_bytes;
    size_t mem_available;

    total_bytes = (maxnum * size);
    if ((maxnum <= 0) || (size <= 0)
        || ((total_bytes / maxnum) != (size_t) size)) {
        return 0;
    }

    mem_available = (context->hidden.mem.stop - context->hidden.mem.here);
    if (total_bytes > mem_available) {
        total_bytes = mem_available;
    }

    SDL_memcpy(ptr, context->hidden.mem.here, total_bytes);
    context->hidden.mem.here += total_bytes;

    return (total_bytes / size);
}

static size_t SDLCALL
mem_write(SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    if ((context->hidden.mem.here + (num * size)) > context->hidden.mem.stop) {
        num = (context->hidden.mem.stop - context->hidden.mem.here) / size;
    }
    SDL_memcpy(context->hidden.mem.here, ptr, num * size);
    context->hidden.mem.here += num * size;
    return num;
}

static size_t SDLCALL
mem_writeconst(SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    SDL_SetError("Can't write to read-only memory");
    return 0;
}

static int SDLCALL
mem_close(SDL_RWops * context)
{
    if (context) {
        SDL_FreeRW(context);
    }
    return 0;
}


/* Functions to create SDL_RWops structures from various data sources */

SDL_RWops *
SDL_RWFromFile(const char *file, const char *mode)
{
    SDL_RWops *rwops = NULL;
    if (!file || !*file || !mode || !*mode) {
        SDL_SetError("SDL_RWFromFile(): No file or no mode specified");
        return NULL;
    }
#if defined(__ANDROID__)
    /* Try to open the file on the filesystem first */
    if (*file == '/') {
        FILE *fp = fopen(file, mode);
        if (fp) {
            return SDL_RWFromFP(fp, SDL_TRUE);
        }
    } else {
        /* Try opening it from internal storage if it's a relative path */
        char *path;
        FILE *fp;

        /* !!! FIXME: why not just "char path[PATH_MAX];" ? */
        path = SDL_stack_alloc(char, PATH_MAX);
        if (path) {
            SDL_snprintf(path, PATH_MAX, "%s/%s",
                         SDL_AndroidGetInternalStoragePath(), file);
            fp = fopen(path, mode);
            SDL_stack_free(path);
            if (fp) {
                return SDL_RWFromFP(fp, SDL_TRUE);
            }
        }
    }

    /* Try to open the file from the asset system */
    rwops = SDL_AllocRW();
    if (!rwops)
        return NULL;            /* SDL_SetError already setup by SDL_AllocRW() */
    if (Android_JNI_FileOpen(rwops, file, mode) < 0) {
        SDL_FreeRW(rwops);
        return NULL;
    }
    rwops->size = Android_JNI_FileSize;
    rwops->seek = Android_JNI_FileSeek;
    rwops->read = Android_JNI_FileRead;
    rwops->write = Android_JNI_FileWrite;
    rwops->close = Android_JNI_FileClose;
    rwops->type = SDL_RWOPS_JNIFILE;

#else
    {
        char *path;
        FILE *fp;

        path = SDL_stack_alloc(char, PATH_MAX);
        if (path) {
            SDL_snprintf(path, PATH_MAX, "assets/%s", file);
            fp = SDL_OpenFPFromBundleOrFallback(path, mode);
            SDL_stack_free(path);
            if (fp == NULL) {
                SDL_SetError("Couldn't open %s", file);
            } else {
                rwops = SDL_RWFromFP(fp, SDL_TRUE);
            }
        }
    }
#endif

    return rwops;
}

SDL_RWops *
SDL_RWFromFP(FILE * fp, SDL_bool autoclose)
{
    SDL_RWops *rwops = NULL;

    rwops = SDL_AllocRW();
    if (rwops != NULL) {
        rwops->size = stdio_size;
        rwops->seek = stdio_seek;
        rwops->read = stdio_read;
        rwops->write = stdio_write;
        rwops->close = stdio_close;
        rwops->hidden.stdio.fp = fp;
        rwops->hidden.stdio.autoclose = autoclose;
        rwops->type = SDL_RWOPS_STDFILE;
    }
    return rwops;
}

SDL_RWops *
SDL_RWFromMem(void *mem, int size)
{
    SDL_RWops *rwops = NULL;
    if (!mem) {
      SDL_InvalidParamError("mem");
      return rwops;
    }
    if (!size) {
      SDL_InvalidParamError("size");
      return rwops;
    }

    rwops = SDL_AllocRW();
    if (rwops != NULL) {
        rwops->size = mem_size;
        rwops->seek = mem_seek;
        rwops->read = mem_read;
        rwops->write = mem_write;
        rwops->close = mem_close;
        rwops->hidden.mem.base = (Uint8 *) mem;
        rwops->hidden.mem.here = rwops->hidden.mem.base;
        rwops->hidden.mem.stop = rwops->hidden.mem.base + size;
        rwops->type = SDL_RWOPS_MEMORY;
    }
    return rwops;
}

SDL_RWops *
SDL_RWFromConstMem(const void *mem, int size)
{
    SDL_RWops *rwops = NULL;
    if (!mem) {
      SDL_InvalidParamError("mem");
      return rwops;
    }
    if (!size) {
      SDL_InvalidParamError("size");
      return rwops;
    }

    rwops = SDL_AllocRW();
    if (rwops != NULL) {
        rwops->size = mem_size;
        rwops->seek = mem_seek;
        rwops->read = mem_read;
        rwops->write = mem_writeconst;
        rwops->close = mem_close;
        rwops->hidden.mem.base = (Uint8 *) mem;
        rwops->hidden.mem.here = rwops->hidden.mem.base;
        rwops->hidden.mem.stop = rwops->hidden.mem.base + size;
        rwops->type = SDL_RWOPS_MEMORY_RO;
    }
    return rwops;
}

SDL_RWops *
SDL_AllocRW(void)
{
    SDL_RWops *area;

    area = (SDL_RWops *) SDL_malloc(sizeof *area);
    if (area == NULL) {
        SDL_OutOfMemory();
    } else {
        area->type = SDL_RWOPS_UNKNOWN;
    }
    return area;
}

void
SDL_FreeRW(SDL_RWops * area)
{
    SDL_free(area);
}

/* Load all the data from an SDL data stream */
void *
SDL_LoadFile_RW(SDL_RWops * src, size_t *datasize, int freesrc)
{
    const int FILE_CHUNK_SIZE = 1024;
    Sint64 size;
    size_t size_read, size_total;
    void *data = NULL, *newdata;

    if (!src) {
        SDL_InvalidParamError("src");
        return NULL;
    }

    size = SDL_RWsize(src);
    if (size < 0) {
        size = FILE_CHUNK_SIZE;
    }
    data = SDL_malloc((size_t)(size + 1));

    size_total = 0;
    for (;;) {
        if ((((Sint64)size_total) + FILE_CHUNK_SIZE) > size) {
            size = (size_total + FILE_CHUNK_SIZE);
            newdata = SDL_realloc(data, (size_t)(size + 1));
            if (!newdata) {
                SDL_free(data);
                data = NULL;
                SDL_OutOfMemory();
                goto done;
            }
            data = newdata;
        }

        size_read = SDL_RWread(src, (char *)data+size_total, 1, (size_t)(size-size_total));
        if (size_read == 0) {
            break;
        }
        size_total += size_read;
    }

    if (datasize) {
        *datasize = size_total;
    }
    ((char *)data)[size_total] = '\0';

done:
    if (freesrc && src) {
        SDL_RWclose(src);
    }
    return data;
}

void *
SDL_LoadFile(const char *file, size_t *datasize)
{
   return SDL_LoadFile_RW(SDL_RWFromFile(file, "rb"), datasize, 1);
}

Sint64
SDL_RWsize(SDL_RWops *context)
{
    return context->size(context);
}

Sint64
SDL_RWseek(SDL_RWops *context, Sint64 offset, int whence)
{
    return context->seek(context, offset, whence);
}

Sint64
SDL_RWtell(SDL_RWops *context)
{
    return context->seek(context, 0, RW_SEEK_CUR);
}

size_t
SDL_RWread(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
    return context->read(context, ptr, size, maxnum);
}

size_t
SDL_RWwrite(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
    return context->write(context, ptr, size, num);
}

int
SDL_RWclose(SDL_RWops *context)
{
    return context->close(context);
}

/* Functions for dynamically reading and writing endian-specific values */

Uint8
SDL_ReadU8(SDL_RWops * src)
{
    Uint8 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return value;
}

Uint16
SDL_ReadLE16(SDL_RWops * src)
{
    Uint16 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapLE16(value);
}

Uint16
SDL_ReadBE16(SDL_RWops * src)
{
    Uint16 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapBE16(value);
}

Uint32
SDL_ReadLE32(SDL_RWops * src)
{
    Uint32 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapLE32(value);
}

Uint32
SDL_ReadBE32(SDL_RWops * src)
{
    Uint32 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapBE32(value);
}

Uint64
SDL_ReadLE64(SDL_RWops * src)
{
    Uint64 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapLE64(value);
}

Uint64
SDL_ReadBE64(SDL_RWops * src)
{
    Uint64 value = 0;

    SDL_RWread(src, &value, sizeof (value), 1);
    return SDL_SwapBE64(value);
}

size_t
SDL_WriteU8(SDL_RWops * dst, Uint8 value)
{
    return SDL_RWwrite(dst, &value, sizeof (value), 1);
}

size_t
SDL_WriteLE16(SDL_RWops * dst, Uint16 value)
{
    const Uint16 swapped = SDL_SwapLE16(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

size_t
SDL_WriteBE16(SDL_RWops * dst, Uint16 value)
{
    const Uint16 swapped = SDL_SwapBE16(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

size_t
SDL_WriteLE32(SDL_RWops * dst, Uint32 value)
{
    const Uint32 swapped = SDL_SwapLE32(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

size_t
SDL_WriteBE32(SDL_RWops * dst, Uint32 value)
{
    const Uint32 swapped = SDL_SwapBE32(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

size_t
SDL_WriteLE64(SDL_RWops * dst, Uint64 value)
{
    const Uint64 swapped = SDL_SwapLE64(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

size_t
SDL_WriteBE64(SDL_RWops * dst, Uint64 value)
{
    const Uint64 swapped = SDL_SwapBE64(value);
    return SDL_RWwrite(dst, &swapped, sizeof (swapped), 1);
}

/* vi: set ts=4 sw=4 expandtab: */
