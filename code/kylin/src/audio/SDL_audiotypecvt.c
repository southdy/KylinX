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
#include "SDL_audio.h"
#include "SDL_audio_c.h"
#include "SDL_cpuinfo.h"
#include "SDL_assert.h"

/* Function pointers set to a CPU-specific implementation. */
SDL_AudioFilter SDL_Convert_S8_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_U8_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_S16_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_U16_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_S32_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S8 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_U8 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S16 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_U16 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S32 = NULL;


#define DIVBY128 0.0078125f
#define DIVBY32768 0.000030517578125f
#define DIVBY8388607 0.00000011920930376163766f


static void SDLCALL
SDL_Convert_S8_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint8 *src = ((const Sint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S8", "AUDIO_F32");

    for (i = cvt->len_cvt; i; --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY128;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U8_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint8 *src = ((const Uint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U8", "AUDIO_F32");

    for (i = cvt->len_cvt; i; --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY128) - 1.0f;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_S16_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint16 *src = ((const Sint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S16", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Sint16); i; --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY32768;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U16_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint16 *src = ((const Uint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U16", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Uint16); i; --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY32768) - 1.0f;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_S32_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint32 *src = (const Sint32 *) cvt->buf;
    float *dst = (float *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S32", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Sint32); i; --i, ++src, ++dst) {
        *dst = ((float) (*src>>8)) * DIVBY8388607;
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S8_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint8 *dst = (Sint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S8");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample >= 1.0f) {
            *dst = 127;
        } else if (sample <= -1.0f) {
            *dst = -128;
        } else {
            *dst = (Sint8)(sample * 127.0f);
        }
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U8_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint8 *dst = (Uint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U8");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample >= 1.0f) {
            *dst = 255;
        } else if (sample <= -1.0f) {
            *dst = 0;
        } else {
            *dst = (Uint8)((sample + 1.0f) * 127.0f);
        }
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S16_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint16 *dst = (Sint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S16");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample >= 1.0f) {
            *dst = 32767;
        } else if (sample <= -1.0f) {
            *dst = -32768;
        } else {
            *dst = (Sint16)(sample * 32767.0f);
        }
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U16_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint16 *dst = (Uint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U16");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample >= 1.0f) {
            *dst = 65535;
        } else if (sample <= -1.0f) {
            *dst = 0;
        } else {
            *dst = (Uint16)((sample + 1.0f) * 32767.0f);
        }
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint32 *dst = (Sint32 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S32");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample >= 1.0f) {
            *dst = 2147483647;
        } else if (sample <= -1.0f) {
            *dst = (Sint32) -2147483648LL;
        } else {
            *dst = ((Sint32)(sample * 8388607.0f)) << 8;
        }
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S32SYS);
    }
}



void SDL_ChooseAudioConverters(void)
{
    static SDL_bool converters_chosen = SDL_FALSE;

    if (converters_chosen) {
        return;
    }

#define SET_CONVERTER_FUNCS(fntype) \
        SDL_Convert_S8_to_F32 = SDL_Convert_S8_to_F32_##fntype; \
        SDL_Convert_U8_to_F32 = SDL_Convert_U8_to_F32_##fntype; \
        SDL_Convert_S16_to_F32 = SDL_Convert_S16_to_F32_##fntype; \
        SDL_Convert_U16_to_F32 = SDL_Convert_U16_to_F32_##fntype; \
        SDL_Convert_S32_to_F32 = SDL_Convert_S32_to_F32_##fntype; \
        SDL_Convert_F32_to_S8 = SDL_Convert_F32_to_S8_##fntype; \
        SDL_Convert_F32_to_U8 = SDL_Convert_F32_to_U8_##fntype; \
        SDL_Convert_F32_to_S16 = SDL_Convert_F32_to_S16_##fntype; \
        SDL_Convert_F32_to_U16 = SDL_Convert_F32_to_U16_##fntype; \
        SDL_Convert_F32_to_S32 = SDL_Convert_F32_to_S32_##fntype; \
        converters_chosen = SDL_TRUE

    SET_CONVERTER_FUNCS(Scalar);

#undef SET_CONVERTER_FUNCS

    SDL_assert(converters_chosen == SDL_TRUE);
}

/* vi: set ts=4 sw=4 expandtab: */
