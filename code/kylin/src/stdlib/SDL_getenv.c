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

#if defined(__ANDROID__)
#include "../core/android/SDL_android.h"
#endif

#include "SDL_stdinc.h"

/* Put a variable into the environment */
/* Note: Name may not contain a '=' character. (Reference: http://www.unix.com/man-page/Linux/3/setenv/) */
int
SDL_setenv(const char *name, const char *value, int overwrite)
{
    /* Input validation */
    if (!name || SDL_strlen(name) == 0 || SDL_strchr(name, '=') != NULL || !value) {
        return (-1);
    }
    
    return setenv(name, value, overwrite);
}

/* Retrieve a variable named "name" from the environment */
char *
SDL_getenv(const char *name)
{
#if defined(__ANDROID__)
    /* Make sure variables from the application manifest are available */
    Android_JNI_GetManifestEnvironmentVariables();
#endif

    /* Input validation */
    if (!name || !*name) {
        return NULL;
    }

    return getenv(name);
}


#ifdef TEST_MAIN
#include <stdio.h>

int
main(int argc, char *argv[])
{
    char *value;

    printf("Checking for non-existent variable... ");
    fflush(stdout);
    if (!SDL_getenv("EXISTS")) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Setting FIRST=VALUE1 in the environment... ");
    fflush(stdout);
    if (SDL_setenv("FIRST", "VALUE1", 0) == 0) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Getting FIRST from the environment... ");
    fflush(stdout);
    value = SDL_getenv("FIRST");
    if (value && (SDL_strcmp(value, "VALUE1") == 0)) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Setting SECOND=VALUE2 in the environment... ");
    fflush(stdout);
    if (SDL_setenv("SECOND", "VALUE2", 0) == 0) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Getting SECOND from the environment... ");
    fflush(stdout);
    value = SDL_getenv("SECOND");
    if (value && (SDL_strcmp(value, "VALUE2") == 0)) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Setting FIRST=NOVALUE in the environment... ");
    fflush(stdout);
    if (SDL_setenv("FIRST", "NOVALUE", 1) == 0) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Getting FIRST from the environment... ");
    fflush(stdout);
    value = SDL_getenv("FIRST");
    if (value && (SDL_strcmp(value, "NOVALUE") == 0)) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    printf("Checking for non-existent variable... ");
    fflush(stdout);
    if (!SDL_getenv("EXISTS")) {
        printf("okay\n");
    } else {
        printf("failed\n");
    }
    return (0);
}
#endif /* TEST_MAIN */

/* vi: set ts=4 sw=4 expandtab: */
