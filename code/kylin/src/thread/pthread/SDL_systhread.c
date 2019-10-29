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

#include "../../SDL_internal.h"
#include "SDL_system.h"

#include <pthread.h>

#if HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif

#include <signal.h>

#if defined(__IPHONEOS__)
#include <dlfcn.h>
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT NULL
#endif
#endif

#include "SDL_log.h"
#include "SDL_platform.h"
#include "SDL_thread.h"
#include "../SDL_thread_c.h"
#include "../SDL_systhread.h"
#ifdef __ANDROID__
#include "../../core/android/SDL_android.h"
#endif

#include "SDL_assert.h"

/* List of signals to mask in the subthreads */
static const int sig_list[] = {
    SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGCHLD, SIGWINCH,
    SIGVTALRM, SIGPROF, 0
};

static void *
RunThread(void *data)
{
#ifdef __ANDROID__
    Android_JNI_SetupThread();
#endif
    SDL_RunThread(data);
    return NULL;
}

#if defined(__IPHONEOS__)
static SDL_bool checked_setname = SDL_FALSE;
static int (*ppthread_setname_np)(const char*) = NULL;
#endif
int
SDL_SYS_CreateThread(SDL_Thread * thread, void *args)
{
    pthread_attr_t type;

    /* do this here before any threads exist, so there's no race condition. */
    #if defined(__IPHONEOS__)
    if (!checked_setname) {
        void *fn = dlsym(RTLD_DEFAULT, "pthread_setname_np");
        ppthread_setname_np = (int(*)(const char*)) fn;
        checked_setname = SDL_TRUE;
    }
    #endif

    /* Set the thread attributes */
    if (pthread_attr_init(&type) != 0) {
        return SDL_SetError("Couldn't initialize pthread attributes");
    }
    pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
    
    /* Set caller-requested stack size. Otherwise: use the system default. */
    if (thread->stacksize) {
        pthread_attr_setstacksize(&type, (size_t) thread->stacksize);
    }

    /* Create the thread and go! */
    if (pthread_create(&thread->handle, &type, RunThread, args) != 0) {
        return SDL_SetError("Not enough resources to create thread");
    }

    return 0;
}

void
SDL_SYS_SetupThread(const char *name)
{
    int i;
    sigset_t mask;

    if (name != NULL) {
        #if defined(__IPHONEOS__)
        SDL_assert(checked_setname);
        if (ppthread_setname_np != NULL) {
            ppthread_setname_np(name);
        }
        #elif HAVE_PTHREAD_SETNAME_NP
            pthread_setname_np(pthread_self(), name);
        #elif HAVE_PTHREAD_SET_NAME_NP
            pthread_set_name_np(pthread_self(), name);
        #endif
    }

    /* Mask asynchronous signals for this thread */
    sigemptyset(&mask);
    for (i = 0; sig_list[i]; ++i) {
        sigaddset(&mask, sig_list[i]);
    }
    pthread_sigmask(SIG_BLOCK, &mask, 0);


#ifdef PTHREAD_CANCEL_ASYNCHRONOUS
    /* Allow ourselves to be asynchronously cancelled */
    {
        int oldstate;
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
    }
#endif
}

SDL_threadID
SDL_ThreadID(void)
{
    return ((SDL_threadID) pthread_self());
}

int
SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority)
{
    struct sched_param sched;
    int policy;
    pthread_t thread = pthread_self();

    if (pthread_getschedparam(thread, &policy, &sched) != 0) {
        return SDL_SetError("pthread_getschedparam() failed");
    }
    if (priority == SDL_THREAD_PRIORITY_LOW) {
        sched.sched_priority = sched_get_priority_min(policy);
    } else if (priority == SDL_THREAD_PRIORITY_TIME_CRITICAL) {
        sched.sched_priority = sched_get_priority_max(policy);
    } else {
        int min_priority = sched_get_priority_min(policy);
        int max_priority = sched_get_priority_max(policy);
        sched.sched_priority = (min_priority + (max_priority - min_priority) / 2);
        if (priority == SDL_THREAD_PRIORITY_HIGH) {
            sched.sched_priority += ((max_priority - min_priority) / 4);
        }
    }
    if (pthread_setschedparam(thread, policy, &sched) != 0) {
        return SDL_SetError("pthread_setschedparam() failed");
    }
    return 0;
}

void
SDL_SYS_WaitThread(SDL_Thread * thread)
{
    pthread_join(thread->handle, 0);
}

void
SDL_SYS_DetachThread(SDL_Thread * thread)
{
    pthread_detach(thread->handle);
}

/* vi: set ts=4 sw=4 expandtab: */
