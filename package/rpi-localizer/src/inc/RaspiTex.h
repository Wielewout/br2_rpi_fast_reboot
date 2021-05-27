/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
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

#ifndef RASPITEX_H_
#define RASPITEX_H_

#include <stdio.h>

#ifndef __MINGW32__
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "interface/mmal/mmal.h"
#endif

#define RASPITEX_VERSION_MAJOR 1
#define RASPITEX_VERSION_MINOR 0

#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1

#define FULL_RES_PREVIEW_FRAME_RATE_NUM 0
#define FULL_RES_PREVIEW_FRAME_RATE_DEN 1

struct RASPITEX_STATE;

/**
 * Contains the internal state and configuration for the GL rendered
 * preview window.
 */
typedef struct RASPITEX_STATE
{
   int version_major;                  /// For binary compatibility
   int version_minor;                  /// Incremented for new features
#ifndef __MINGW32__
   MMAL_PORT_T *preview_port;          /// Source port for preview opaque buffers
   MMAL_POOL_T *preview_pool;          /// Pool for storing opaque buffer handles
   MMAL_QUEUE_T *preview_queue;        /// Queue preview buffers to display in order
   VCOS_THREAD_T preview_thread;       /// Preview worker / GL rendering thread
#endif
   uint32_t preview_stop;              /// If zero the worker can continue

   /* Copy of preview window params */
   int32_t preview_x;                  /// x-offset of preview window
   int32_t preview_y;                  /// y-offset of preview window
   int32_t preview_width;              /// preview y-plane width in pixels
   int32_t preview_height;             /// preview y-plane height in pixels

   /* Display rectangle for the native window */
   int32_t x;                          /// x-offset in pixels
   int32_t y;                          /// y-offset in pixels
   int32_t width;                      /// width in pixels
   int32_t height;                     /// height in pixels
   int opacity;                        /// Alpha value for display element
   int gl_win_defined;                 /// Use rect from --glwin instead of preview

#ifndef __MINGW32__
   /* DispmanX info. This might be unused if a custom create_native_window
    * does something else. */
   DISPMANX_DISPLAY_HANDLE_T disp;     /// Dispmanx display for GL preview
   EGL_DISPMANX_WINDOW_T win;          /// Dispmanx handle for preview surface

   EGLNativeWindowType* native_window; /// Native window used for EGL surface
   EGLDisplay display;                 /// The current EGL display
   EGLSurface surface;                 /// The current EGL surface
   EGLContext context;                 /// The current EGL context
   const EGLint *egl_config_attribs;   /// GL scenes preferred EGL configuration

   GLuint texture[4];                  /// The Y plane texture and background texture and difference texture
   GLuint frameBuffers[4];
   int    current_texture;
   EGLImageKHR egl_image;            /// EGL image for Y plane texture

   MMAL_BUFFER_HEADER_T *preview_buf;  /// MMAL buffer currently bound to texture(s)
   MMAL_BUFFER_HEADER_T *current_buf;
#endif

   void *scene_state;                  /// Pointer to scene specific data
   int save_image;
   int save_image_warmup;
   float luminence_thresh;
   int led_blob_size;
   int led_one_zero_thresh;
   uint32_t led_find_radius;
   uint16_t led_radius;
   int verbose;                        /// Log FPS
   uint8_t is_ready;
   float prev_buff_time;
   float curr_buff_time;

} RASPITEX_STATE;

int raspitex_init(RASPITEX_STATE *state);
#ifndef __MINGW32__
void raspitex_destroy(RASPITEX_STATE *state);
int raspitex_start(RASPITEX_STATE *state);
void raspitex_stop(RASPITEX_STATE *state);
void raspitex_set_defaults(RASPITEX_STATE *state);
int raspitex_configure_preview_port(RASPITEX_STATE *state,
      MMAL_PORT_T *preview_port);
#endif
void raspitex_display_help();
int raspitex_parse_cmdline(RASPITEX_STATE *state,
      const char *arg1, const char *arg2);

#endif /* RASPITEX_H_ */
