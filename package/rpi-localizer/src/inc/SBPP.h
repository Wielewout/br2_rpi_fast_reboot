/*
Copyright (c) 2018, Hassaan Janjua
All rights reserved.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SBPP_H
#define SBPP_H

#include "RaspiTex.h"

#define BITS_PER_BYTE       8
#define BITS_PER_RGBA_PIXEL 8

#define SETUP_FPS \
\
    struct timespec __start_time; \
    struct timespec __prev_time; \
    struct timespec __time_difference; \
    const char *__msg; \
    unsigned int __frames = 0; \
    unsigned int __interval = 0; \
    struct timespec __gettime_now;

#define START_FPS(msg, interval) \
    clock_gettime(CLOCK_REALTIME, &__gettime_now); \
    __prev_time = __start_time = __gettime_now; \
    __frames = 0; \
    __interval = interval; \
    __msg = msg;

int sbpp_open(RASPITEX_STATE *state);
int sbpp_redraw(RASPITEX_STATE *raspitex_state);
int sbpp_init(RASPITEX_STATE *state);

#endif /* SBPP_H */
