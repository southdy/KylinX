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
#include "./SDL_internal.h"

#include <unistd.h> /* For _exit(), etc. */

/* Initialization code for SDL */

#include "SDL.h"
#include "SDL_bits.h"
#include "SDL_revision.h"
#include "SDL_assert_c.h"
#include "events/SDL_events_c.h"
#include "sensor/SDL_sensor_c.h"

/* Initialization/Cleanup routines */
#include "timer/SDL_timer_c.h"


/* This is not declared in any header, although it is shared between some
    parts of SDL, because we don't want anything calling it without an
    extremely good reason. */
extern SDL_NORETURN void SDL_ExitProcess(int exitcode);
SDL_NORETURN void SDL_ExitProcess(const int exitcode)
{
    _exit(exitcode);
}


/* The initialized subsystems */
#ifdef SDL_MAIN_NEEDED
static SDL_bool SDL_MainIsReady = SDL_FALSE;
#else
static SDL_bool SDL_MainIsReady = SDL_TRUE;
#endif
static SDL_bool SDL_bInMainQuit = SDL_FALSE;
static Uint8 SDL_SubsystemRefCount[ 32 ];

/* Private helper to increment a subsystem's ref counter. */
static void
SDL_PrivateSubsystemRefCountIncr(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    SDL_assert(SDL_SubsystemRefCount[subsystem_index] < 255);
    ++SDL_SubsystemRefCount[subsystem_index];
}

/* Private helper to decrement a subsystem's ref counter. */
static void
SDL_PrivateSubsystemRefCountDecr(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    if (SDL_SubsystemRefCount[subsystem_index] > 0) {
        --SDL_SubsystemRefCount[subsystem_index];
    }
}

/* Private helper to check if a system needs init. */
static SDL_bool
SDL_PrivateShouldInitSubsystem(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    SDL_assert(SDL_SubsystemRefCount[subsystem_index] < 255);
    return (SDL_SubsystemRefCount[subsystem_index] == 0) ? SDL_TRUE : SDL_FALSE;
}

/* Private helper to check if a system needs to be quit. */
static SDL_bool
SDL_PrivateShouldQuitSubsystem(Uint32 subsystem) {
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    if (SDL_SubsystemRefCount[subsystem_index] == 0) {
      return SDL_FALSE;
    }

    /* If we're in SDL_Quit, we shut down every subsystem, even if refcount
     * isn't zero.
     */
    return (SDL_SubsystemRefCount[subsystem_index] == 1 || SDL_bInMainQuit) ? SDL_TRUE : SDL_FALSE;
}

void
SDL_SetMainReady(void)
{
    SDL_MainIsReady = SDL_TRUE;
}

int
SDL_InitSubSystem(Uint32 flags)
{
    if (!SDL_MainIsReady) {
        SDL_SetError("Application didn't initialize properly, did you include SDL_main.h in the file containing your main() function?");
        return -1;
    }

    /* Clear the error message */
    SDL_ClearError();

    if ((flags & SDL_INIT_VIDEO)) {
        /* video or joystick implies events */
        flags |= SDL_INIT_EVENTS;
    }

    SDL_TicksInit();

    /* Initialize the event subsystem */
    if ((flags & SDL_INIT_EVENTS)) {
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_EVENTS)) {
            if (SDL_EventsInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_EVENTS);
    }

    /* Initialize the timer subsystem */
    if ((flags & SDL_INIT_TIMER)) {
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_TIMER)) {
            if (SDL_TimerInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_TIMER);
    }

    /* Initialize the video subsystem */
    if ((flags & SDL_INIT_VIDEO)) {
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_VIDEO)) {
            if (SDL_VideoInit(NULL) < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_VIDEO);
    }

    /* Initialize the audio subsystem */
    if ((flags & SDL_INIT_AUDIO)) {
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_AUDIO)) {
            if (SDL_AudioInit(NULL) < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_AUDIO);
    }

    /* Initialize the sensor subsystem */
    if ((flags & SDL_INIT_SENSOR)) {
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_SENSOR)) {
            if (SDL_SensorInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_SENSOR);
    }

    return (0);
}

int
SDL_Init(Uint32 flags)
{
    return SDL_InitSubSystem(flags);
}

void
SDL_QuitSubSystem(Uint32 flags)
{
    /* Shut down requested initialized subsystems */
    if ((flags & SDL_INIT_SENSOR)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_SENSOR)) {
            SDL_SensorQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_SENSOR);
    }

    if ((flags & SDL_INIT_AUDIO)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_AUDIO)) {
            SDL_AudioQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_AUDIO);
    }

    if ((flags & SDL_INIT_VIDEO)) {
        /* video implies events */
        flags |= SDL_INIT_EVENTS;

        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_VIDEO)) {
            SDL_VideoQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_VIDEO);
    }

    if ((flags & SDL_INIT_TIMER)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_TIMER)) {
            SDL_TimerQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_TIMER);
    }

    if ((flags & SDL_INIT_EVENTS)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_EVENTS)) {
            SDL_EventsQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_EVENTS);
    }
}

Uint32
SDL_WasInit(Uint32 flags)
{
    int i;
    int num_subsystems = SDL_arraysize(SDL_SubsystemRefCount);
    Uint32 initialized = 0;

    /* Fast path for checking one flag */
    if (SDL_HasExactlyOneBitSet32(flags)) {
        int subsystem_index = SDL_MostSignificantBitIndex32(flags);
        return SDL_SubsystemRefCount[subsystem_index] ? flags : 0;
    }

    if (!flags) {
        flags = SDL_INIT_EVERYTHING;
    }

    num_subsystems = SDL_min(num_subsystems, SDL_MostSignificantBitIndex32(flags) + 1);

    /* Iterate over each bit in flags, and check the matching subsystem. */
    for (i = 0; i < num_subsystems; ++i) {
        if ((flags & 1) && SDL_SubsystemRefCount[i] > 0) {
            initialized |= (1 << i);
        }

        flags >>= 1;
    }

    return initialized;
}

void
SDL_Quit(void)
{
    SDL_bInMainQuit = SDL_TRUE;

    /* Quit all subsystems */
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);

    SDL_TicksQuit();

    SDL_ClearHints();
    SDL_AssertionsQuit();
    SDL_LogResetPriorities();

    /* Now that every subsystem has been quit, we reset the subsystem refcount
     * and the list of initialized subsystems.
     */
    SDL_memset( SDL_SubsystemRefCount, 0x0, sizeof(SDL_SubsystemRefCount) );

    SDL_bInMainQuit = SDL_FALSE;
}

/* Get the library version number */
void
SDL_GetVersion(SDL_version * ver)
{
    SDL_VERSION(ver);
}

/* Get the library source revision */
const char *
SDL_GetRevision(void)
{
    return SDL_REVISION;
}

/* Get the library source revision number */
int
SDL_GetRevisionNumber(void)
{
    return SDL_REVISION_NUMBER;
}

/* Get the name of the platform */
const char *
SDL_GetPlatform()
{
#if __ANDROID__
    return "Android";
#elif __IPHONEOS__
    return "iOS";
#else
    return "Unknown (see SDL_platform.h)";
#endif
}

SDL_bool
SDL_IsTablet()
{
#if __ANDROID__
    extern SDL_bool SDL_IsAndroidTablet(void);
    return SDL_IsAndroidTablet();
#elif __IPHONEOS__
    extern SDL_bool SDL_IsIPad(void);
    return SDL_IsIPad();
#else
    return SDL_FALSE;
#endif
}

/* vi: set sts=4 ts=4 sw=4 expandtab: */
