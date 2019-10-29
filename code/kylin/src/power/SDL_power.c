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
#include "SDL_power.h"
#include "SDL_syspower.h"

/*
 * Returns SDL_TRUE if we have a definitive answer.
 * SDL_FALSE to try next implementation.
 */
typedef SDL_bool
    (*SDL_GetPowerInfo_Impl) (SDL_PowerState * state, int *seconds,
                              int *percent);

static SDL_GetPowerInfo_Impl implementations[] = {
#ifdef SDL_POWER_UIKIT          /* handles iPhone/iPad/etc */
    SDL_GetPowerInfo_UIKit,
#endif
#ifdef SDL_POWER_ANDROID        /* handles Android. */
    SDL_GetPowerInfo_Android,
#endif
};

SDL_PowerState
SDL_GetPowerInfo(int *seconds, int *percent)
{
    const int total = sizeof(implementations) / sizeof(implementations[0]);
    SDL_PowerState retval = SDL_POWERSTATE_UNKNOWN;
    int i;

    int _seconds, _percent;
    /* Make these never NULL for platform-specific implementations. */
    if (seconds == NULL) {
        seconds = &_seconds;
    }
    if (percent == NULL) {
        percent = &_percent;
    }

    for (i = 0; i < total; i++) {
        if (implementations[i](&retval, seconds, percent)) {
            return retval;
        }
    }

    /* nothing was definitive. */
    *seconds = -1;
    *percent = -1;
    return SDL_POWERSTATE_UNKNOWN;
}

/* vi: set ts=4 sw=4 expandtab: */
