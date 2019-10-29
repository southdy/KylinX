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
 *  \file SDL_system.h
 *
 *  Include file for platform specific SDL API functions
 */

#ifndef SDL_system_h_
#define SDL_system_h_

#include "SDL_stdinc.h"
#include "SDL_keyboard.h"
#include "SDL_render.h"
#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif


/* Platform specific functions for iOS */
#if defined(__IPHONEOS__) && __IPHONEOS__

#define SDL_iOSSetAnimationCallback(window, interval, callback, callbackParam) SDL_iPhoneSetAnimationCallback(window, interval, callback, callbackParam)
extern DECLSPEC int SDLCALL SDL_iPhoneSetAnimationCallback(SDL_Window * window, int interval, void (*callback)(void*), void *callbackParam);

#define SDL_iOSSetEventPump(enabled) SDL_iPhoneSetEventPump(enabled)
extern DECLSPEC void SDLCALL SDL_iPhoneSetEventPump(SDL_bool enabled);

#endif /* __IPHONEOS__ */


/* Platform specific functions for Android */
#if defined(__ANDROID__) && __ANDROID__

/**
   \brief Get the JNI environment for the current thread

   This returns JNIEnv*, but the prototype is void* so we don't need jni.h
 */
extern DECLSPEC void * SDLCALL SDL_AndroidGetJNIEnv(void);

/**
   \brief Get the SDL Activity object for the application

   This returns jobject, but the prototype is void* so we don't need jni.h
   The jobject returned by SDL_AndroidGetActivity is a local reference.
   It is the caller's responsibility to properly release it
   (using env->Push/PopLocalFrame or manually with env->DeleteLocalRef)
 */
extern DECLSPEC void * SDLCALL SDL_AndroidGetActivity(void);

/**
   \brief Return true if the application is running on Android TV
 */
extern DECLSPEC SDL_bool SDLCALL SDL_IsAndroidTV(void);

/**
   \brief Return true if the application is running on a Chromebook
 */
extern DECLSPEC SDL_bool SDLCALL SDL_IsChromebook(void);

/**
  \brief Return true is the application is running on a Samsung DeX docking station
 */
extern DECLSPEC SDL_bool SDLCALL SDL_IsDeXMode(void);

/**
 \brief Trigger the Android system back button behavior.
 */
extern DECLSPEC void SDLCALL SDL_AndroidBackButton(void);

/**
   See the official Android developer guide for more information:
   http://developer.android.com/guide/topics/data/data-storage.html
*/
#define SDL_ANDROID_EXTERNAL_STORAGE_READ   0x01
#define SDL_ANDROID_EXTERNAL_STORAGE_WRITE  0x02

/**
   \brief Get the path used for internal storage for this application.

   This path is unique to your application and cannot be written to
   by other applications.
 */
extern DECLSPEC const char * SDLCALL SDL_AndroidGetInternalStoragePath(void);

/**
   \brief Get the current state of external storage, a bitmask of these values:
    SDL_ANDROID_EXTERNAL_STORAGE_READ
    SDL_ANDROID_EXTERNAL_STORAGE_WRITE

   If external storage is currently unavailable, this will return 0.
*/
extern DECLSPEC int SDLCALL SDL_AndroidGetExternalStorageState(void);

/**
   \brief Get the path used for external storage for this application.

   This path is unique to your application, but is public and can be
   written to by other applications.
 */
extern DECLSPEC const char * SDLCALL SDL_AndroidGetExternalStoragePath(void);

#endif /* __ANDROID__ */

/**
 \brief Return true if the current device is a tablet.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_IsTablet(void);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_system_h_ */

/* vi: set ts=4 sw=4 expandtab: */
