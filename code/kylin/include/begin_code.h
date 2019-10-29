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

/**
 *  \file begin_code.h
 *
 *  This file sets things up for C dynamic library function definitions,
 *  static inlined functions, and structures aligned at 4-byte alignment.
 *  If you don't like ugly C preprocessor code, don't look at this file. :)
 */

/* This shouldn't be nested -- included it around code only. */
#ifdef _begin_code_h
#error Nested inclusion of begin_code.h
#endif
#define _begin_code_h

#ifndef SDL_DEPRECATED
#  if (__GNUC__ >= 4)  /* technically, this arrived in gcc 3.1, but oh well. */
#    define SDL_DEPRECATED __attribute__((deprecated))
#  else
#    define SDL_DEPRECATED
#  endif
#endif

#ifndef SDL_UNUSED
#  ifdef __GNUC__
#    define SDL_UNUSED __attribute__((unused))
#  else
#    define SDL_UNUSED
#  endif
#endif

/* Some compilers use a special export keyword */
#ifndef DECLSPEC
# if defined(__WIN32__) || defined(__WINRT__)
#  ifdef __BORLANDC__
#   ifdef BUILD_SDL
#    define DECLSPEC
#   else
#    define DECLSPEC    __declspec(dllimport)
#   endif
#  else
#   define DECLSPEC __declspec(dllexport)
#  endif
# elif defined(__OS2__)
#   ifdef BUILD_SDL
#    define DECLSPEC    __declspec(dllexport)
#   else
#    define DECLSPEC
#   endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define DECLSPEC __attribute__ ((visibility("default")))
#  else
#   define DECLSPEC
#  endif
# endif
#endif

/* By default SDL uses the C calling convention */
#ifndef SDLCALL
#if (defined(__WIN32__) || defined(__WINRT__)) && !defined(__GNUC__)
#define SDLCALL __cdecl
#elif defined(__OS2__) || defined(__EMX__)
#define SDLCALL _System
# if defined (__GNUC__) && !defined(_System)
#  define _System /* for old EMX/GCC compat.  */
# endif
#else
#define SDLCALL
#endif
#endif /* SDLCALL */

#ifndef SDL_INLINE
#if defined(__GNUC__)
#define SDL_INLINE __inline__
#elif defined(_MSC_VER) || defined(__BORLANDC__) || \
      defined(__DMC__) || defined(__SC__) || \
      defined(__WATCOMC__) || defined(__LCC__) || \
      defined(__DECC) || defined(__CC_ARM)
#define SDL_INLINE __inline
#ifndef __inline__
#define __inline__ __inline
#endif
#else
#define SDL_INLINE inline
#ifndef __inline__
#define __inline__ inline
#endif
#endif
#endif /* SDL_INLINE not defined */

#ifndef SDL_FORCE_INLINE
#if defined(_MSC_VER)
#define SDL_FORCE_INLINE __forceinline
#elif ( (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__) )
#define SDL_FORCE_INLINE __attribute__((always_inline)) static __inline__
#else
#define SDL_FORCE_INLINE static SDL_INLINE
#endif
#endif /* SDL_FORCE_INLINE not defined */

#ifndef SDL_NORETURN
#if defined(__GNUC__)
#define SDL_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define SDL_NORETURN __declspec(noreturn)
#else
#define SDL_NORETURN
#endif
#endif /* SDL_NORETURN not defined */

/* Apparently this is needed by several Windows compilers */
#if !defined(__MACH__)
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif /* NULL */
#endif /* ! Mac OS X - breaks precompiled headers */
