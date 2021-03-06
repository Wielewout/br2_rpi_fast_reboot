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

#include "RaspiTexUtil.h"
#include "RaspiTex.h"
#include "LedDetector.h"
#include <bcm_host.h>
#include <GLES2/gl2.h>

VCOS_LOG_CAT_T raspitex_log_category;

/**
 * \file RaspiTexUtil.c
 *
 * Provides default implementations for the raspitex_scene_ops functions
 * and general utility functions.
 */

/**
 * Deletes textures and EGL surfaces and context.
 * @param   raspitex_state  Pointer to the Raspi
 */
void raspitexutil_gl_term(RASPITEX_STATE *raspitex_state)
{
   vcos_log_trace("%s", VCOS_FUNCTION);

   glDeleteTextures(3, raspitex_state->texture);
   eglDestroyImageKHR(raspitex_state->display, raspitex_state->egl_image);
   raspitex_state->egl_image = EGL_NO_IMAGE_KHR;

   /* Terminate EGL */
   eglMakeCurrent(raspitex_state->display, EGL_NO_SURFACE,
         EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(raspitex_state->display, raspitex_state->context);
   eglDestroySurface(raspitex_state->display, raspitex_state->surface);
   eglTerminate(raspitex_state->display);
}

/** Creates a native window for the GL surface using dispmanx
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful, otherwise, -1 is returned.
 */
int raspitexutil_create_native_window(RASPITEX_STATE *raspitex_state)
{
   VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};
   VC_RECT_T src_rect = {0};
   VC_RECT_T dest_rect = {0};
   uint32_t disp_num = 0; // Primary
   uint32_t layer_num = 0;
   DISPMANX_ELEMENT_HANDLE_T elem;
   DISPMANX_UPDATE_HANDLE_T update;

   alpha.opacity = raspitex_state->opacity;
   dest_rect.x = raspitex_state->x;
   dest_rect.y = raspitex_state->y;
   dest_rect.width = raspitex_state->width;
   dest_rect.height = raspitex_state->height/16;

   vcos_log_trace("%s: %d,%d,%d,%d %d,%d,0x%x,0x%x", VCOS_FUNCTION,
         src_rect.x, src_rect.y, src_rect.width, src_rect.height,
         dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height);

   src_rect.width = (dest_rect.width) << 16;
   src_rect.height = dest_rect.height/16 << 16;

   raspitex_state->disp = vc_dispmanx_display_open(disp_num);
   if (raspitex_state->disp == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to open display handle");
      goto error;
   }

   update = vc_dispmanx_update_start(0);
   if (update == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to open update handle");
      goto error;
   }

   elem = vc_dispmanx_element_add(update, raspitex_state->disp, layer_num,
         &dest_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, NULL,
         DISPMANX_NO_ROTATE);
   if (elem == DISPMANX_NO_HANDLE)
   {
      vcos_log_error("Failed to create element handle");
      goto error;
   }

   raspitex_state->win.element = elem;
   raspitex_state->win.width = raspitex_state->width;
   raspitex_state->win.height = raspitex_state->height/16;
   vc_dispmanx_update_submit_sync(update);

   raspitex_state->native_window = (EGLNativeWindowType*) &raspitex_state->win;

   return 0;
error:
   return -1;
}

/** Destroys the pools of buffers used by the GL renderer.
 * @param raspitex_state A pointer to the GL preview state.
 */
void raspitexutil_destroy_native_window(RASPITEX_STATE *raspitex_state)
{
   vcos_log_trace("%s", VCOS_FUNCTION);
   if (raspitex_state->disp != DISPMANX_NO_HANDLE)
   {
      vc_dispmanx_display_close(raspitex_state->disp);
      raspitex_state->disp = DISPMANX_NO_HANDLE;
   }
}

/** Creates the EGL context and window surface for the native window
 * using specified arguments.
 * @param raspitex_state  A pointer to the GL preview state. This contains
 *                        the native_window pointer.
 * @param attribs         The config attributes.
 * @param context_attribs The context attributes.
 * @return Zero if successful.
 */
static int raspitexutil_gl_common(RASPITEX_STATE *raspitex_state,
      const EGLint attribs[], const EGLint context_attribs[])
{
   EGLConfig config;
   EGLint num_configs;

   vcos_log_trace("%s", VCOS_FUNCTION);

   if (raspitex_state->native_window == NULL)
   {
      vcos_log_error("%s: No native window", VCOS_FUNCTION);
      goto error;
   }

   raspitex_state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (raspitex_state->display == EGL_NO_DISPLAY)
   {
      vcos_log_error("%s: Failed to get EGL display", VCOS_FUNCTION);
      goto error;
   }

   if (! eglInitialize(raspitex_state->display, 0, 0))
   {
      vcos_log_error("%s: eglInitialize failed", VCOS_FUNCTION);
      goto error;
   }

   if (! eglChooseConfig(raspitex_state->display, attribs, &config,
            1, &num_configs))
   {
      vcos_log_error("%s: eglChooseConfig failed", VCOS_FUNCTION);
      goto error;
   }

   raspitex_state->surface = eglCreateWindowSurface(raspitex_state->display,
         config, raspitex_state->native_window, NULL);
   if (raspitex_state->surface == EGL_NO_SURFACE)
   {
      vcos_log_error("%s: eglCreateWindowSurface failed", VCOS_FUNCTION);
      goto error;
   }

   raspitex_state->context = eglCreateContext(raspitex_state->display,
         config, EGL_NO_CONTEXT, context_attribs);
   if (raspitex_state->context == EGL_NO_CONTEXT)
   {
      vcos_log_error("%s: eglCreateContext failed", VCOS_FUNCTION);
      goto error;
   }

   if (!eglMakeCurrent(raspitex_state->display, raspitex_state->surface,
            raspitex_state->surface, raspitex_state->context))
   {
      vcos_log_error("%s: Failed to activate EGL context", VCOS_FUNCTION);
      goto error;
   }

   return 0;

error:
   vcos_log_error("%s: EGL error 0x%08x", VCOS_FUNCTION, eglGetError());
   raspitexutil_gl_term(raspitex_state);
   return -1;
}

/* Creates the RGBA and luma textures with some default parameters
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int raspitexutil_create_textures(RASPITEX_STATE *raspitex_state)
{
  GLCHK(glGenTextures(4, raspitex_state->texture));
  raspitex_state->current_texture = 0;

/*
  uint8_t *d = (uint8_t *)malloc(1920 * 1088 * 4);
  memset(d, 0xFF, 1920 * 1088 * 4);

  GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->texture[2]));
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1088, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
  GLCHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST));
  GLCHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST));
  glBindTexture(GL_TEXTURE_2D, 0);

  GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->texture[3]));
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1088, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
  free(d);
  GLCHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST));
  GLCHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST));
  glBindTexture(GL_TEXTURE_2D, 0);

  GLCHK(glGenFramebuffers(4, raspitex_state->frameBuffers));

  GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->frameBuffers[2]));
    GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raspitex_state->texture[2], 0));
    GLCHK(glViewport ( 0, 0, 1920, 1088 ));
  GLCHK(glBindFramebuffer(GL_FRAMEBUFFER,0));

  GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->frameBuffers[3]));
    GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raspitex_state->texture[3], 0));
    GLCHK(glViewport ( 0, 0, 1920, 1088 ));
  GLCHK(glBindFramebuffer(GL_FRAMEBUFFER,0));
*/
   return 0;
}

/**
 * Creates an OpenGL ES 2.X context.
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int raspitexutil_gl_init_2_0(RASPITEX_STATE *raspitex_state)
{
   int rc;
   const EGLint* attribs = raspitex_state->egl_config_attribs;;

   const EGLint default_attribs[] =
   {
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };

   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   if (! attribs)
      attribs = default_attribs;

   vcos_log_trace("%s", VCOS_FUNCTION);
   rc = raspitexutil_gl_common(raspitex_state, attribs, context_attribs);
   if (rc != 0)
      goto end;

   rc = raspitexutil_create_textures(raspitex_state);
end:
   return rc;
}

/**
 * Advances the texture and EGL image to the next MMAL buffer.
 *
 * @param display The EGL display.
 * @param target The EGL image target e.g. EGL_IMAGE_BRCM_MULTIMEDIA
 * @param mm_buf The EGL client buffer (mmal opaque buffer) that is used to
 * create the EGL Image for the preview texture.
 * @param egl_image Pointer to the EGL image to update with mm_buf.
 * @param texture Pointer to the texture to update from EGL image.
 * @return Zero if successful.
 */
int raspitexutil_do_update_texture(EGLDisplay display, EGLenum target,
      EGLClientBuffer mm_buf, GLuint *texture, EGLImageKHR *egl_image)
{
   vcos_log_trace("%s: mm_buf %u", VCOS_FUNCTION, (unsigned) mm_buf);
   GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, *texture));

   if (*egl_image != EGL_NO_IMAGE_KHR)
   {
      /* Discard the EGL image for the preview frame */
      eglDestroyImageKHR(display, *egl_image);
      *egl_image = EGL_NO_IMAGE_KHR;
   }

   *egl_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, target, mm_buf, NULL);
   GLCHK(glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, *egl_image));
 
   return 0;
}

extern led_detector g_led_dectector;
int bg_counter = 0;
int bg_ready = 0;
/**
 * Updates the Y plane texture to the specified MMAL buffer.
 * @param raspitex_state A pointer to the GL preview state.
 * @param mm_buf The MMAL buffer.
 * @return Zero if successful.
 */
int raspitexutil_update_texture(RASPITEX_STATE *raspitex_state,
      EGLClientBuffer mm_buf) 
{
  //raspitex_state->current_texture = !raspitex_state->current_texture;

  int ret = raspitexutil_do_update_texture(raspitex_state->display, 
            EGL_IMAGE_BRCM_MULTIMEDIA_Y, mm_buf,
            &raspitex_state->texture[0], &raspitex_state->egl_image);

  if (bg_counter % 1200 == 50) {
    bg_ready = 1;
  }

  if (bg_ready && g_led_dectector.leds_queue_size == 0) {
    ret = raspitexutil_do_update_texture(raspitex_state->display,
                EGL_IMAGE_BRCM_MULTIMEDIA_Y, mm_buf,
                &raspitex_state->texture[1], &raspitex_state->egl_image);
    fprintf(stdout, "updating bg\n");
    fflush(stdout);

    bg_counter = 50;
    bg_ready = 0;
  }
  bg_counter++;

  return ret;
}

/**
 * Takes a description of shader program, compiles it and gets the locations
 * of uniforms and attributes.
 *
 * @param p The shader program state.
 * @return Zero if successful.
 */
int raspitexutil_build_shader_program(RASPITEXUTIL_SHADER_PROGRAM_T *p)
{
    GLint status;
    int i = 0;
    char log[1024];
    int logLen = 0;
    vcos_assert(p);
    vcos_assert(p->vertex_source);
    vcos_assert(p->fragment_source);
    
    if (! (p && p->vertex_source && p->fragment_source))
        goto fail;

    p->vs = p->fs = 0;

    p->vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(p->vs, 1, &p->vertex_source, NULL);
    GLCHK(glCompileShader(p->vs));
    glGetShaderiv(p->vs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->vs, sizeof(log), &logLen, log);
        vcos_log_error("Program info log %s", log);
        goto fail;
    }

    p->fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(p->fs, 1, &p->fragment_source, NULL);
    glCompileShader(p->fs);

    glGetShaderiv(p->fs, GL_COMPILE_STATUS, &status);
    if (! status) {
        glGetShaderInfoLog(p->fs, sizeof(log), &logLen, log);
        vcos_log_error("Program info log %s", log);
        goto fail;
    }

    p->program = glCreateProgram();
    glAttachShader(p->program, p->vs);
    glAttachShader(p->program, p->fs);
    glLinkProgram(p->program);
    glGetProgramiv(p->program, GL_LINK_STATUS, &status);
    if (! status)
    {
        vcos_log_error("Failed to link shader program");
        glGetProgramInfoLog(p->program, sizeof(log), &logLen, log);
        vcos_log_error("Program info log %s", log);
        goto fail;
    }

    for (i = 0; i < SHADER_MAX_ATTRIBUTES; ++i)
    {
        if (! p->attribute_names[i])
            break;
        p->attribute_locations[i] = glGetAttribLocation(p->program, p->attribute_names[i]);
        if (p->attribute_locations[i] == -1)
        {
            vcos_log_error("Failed to get location for attribute %s",
                  p->attribute_names[i]);
            goto fail;
        }
        else {
            vcos_log_trace("Attribute for %s is %d",
                  p->attribute_names[i], p->attribute_locations[i]);
        }
    }

    for (i = 0; i < SHADER_MAX_UNIFORMS; ++i)
    {
        if (! p->uniform_names[i])
            break;
        p->uniform_locations[i] = glGetUniformLocation(p->program, p->uniform_names[i]);
        if (p->uniform_locations[i] == -1)
        {
            vcos_log_error("Failed to get location for uniform %s",
                  p->uniform_names[i]);
            goto fail;
        }
        else {
            vcos_log_trace("Uniform for %s is %d",
                  p->uniform_names[i], p->uniform_locations[i]);
        }
    }

    return 0;

fail:
    vcos_log_error("%s: Failed to build shader program", VCOS_FUNCTION);
    if (p)
    {
        glDeleteProgram(p->program);
        glDeleteShader(p->fs);
        glDeleteShader(p->vs);
    }
    return -1;
}

