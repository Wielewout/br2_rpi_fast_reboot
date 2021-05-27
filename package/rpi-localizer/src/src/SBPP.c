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

#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "lodepng.h"
#include "LedDetector.h"
#include "SBPP.h"


#define SAVE_PNG 0
extern uint32_t led_found;

/* Draw a scaled quad showing the the entire texture with the
 * origin defined as an attribute */
static RASPITEXUTIL_SHADER_PROGRAM_T sbpp_shader =
{
    .vertex_source =
    "attribute vec2 vertex;\n"
    "attribute vec2 top_left;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "   texcoord = vertex + vec2(0.0, 1.0);\n"
    "   gl_Position = vec4(top_left + vertex, 0.0, 0.5);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "    vec4 c = texture2D(tex, texcoord);\n"
    "    if (c.b == c.r)\n"
    "      gl_FragColor = vec4(c.b, c.b, c.b, 1.0);\n"
    "    else\n"
    "      gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);\n"
    "}\n",
    .uniform_names = {"tex", "tex2"},
    .attribute_names = {"vertex", "top_left"},
};

static RASPITEXUTIL_SHADER_PROGRAM_T diff_shader =
{
    .vertex_source =
    "attribute vec2 vertex;\n"
    "attribute vec2 top_left;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "   texcoord = vertex + vec2(0.0, 1.0);\n"
    "   gl_Position = vec4(top_left + vertex, 0.0, 0.5);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "uniform samplerExternalOES tex2;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "  float f = abs(texture2D(tex, texcoord).r - texture2D(tex2, texcoord).r) ; "
    "  if (texcoord.y > 0.5 ) {\n"
    "    gl_FragColor = texture2D(tex, texcoord);\n"
    "  } else {\n"
    "    gl_FragColor = texture2D(tex2, texcoord);\n"
    "  }\n"
    "}\n",
    .uniform_names = {"tex", "tex2"},
    .attribute_names = {"vertex", "top_left"},
};

static RASPITEXUTIL_SHADER_PROGRAM_T copy_shader =
{
    .vertex_source =
    "attribute vec2 vertex;\n"
    "attribute vec2 top_left;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "   texcoord = vertex + vec2(0.0, 1.0);\n"
    "   gl_Position = vec4(top_left + vertex, 0.0, 0.5);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "    gl_FragColor = texture2D(tex, texcoord);\n"
    "}\n",
    .uniform_names = {"tex"},
    .attribute_names = {"vertex", "top_left"},
};

static GLfloat varray[] =
{
   0.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
   1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};

static const EGLint sbpp_egl_config_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

unsigned char *data;

#ifdef LOC_ENABLE_SAVE_IMAGE
unsigned char *image;
unsigned char *image_data;
#endif /* LOC_ENABLE_SAVE_IMAGE */

int image_counter = 0;

led_detector g_led_dectector;

#if LOCALIZATION_DEBUG > 0

SETUP_FPS

#ifdef LOC_ENABLE_SAVE_IMAGE
/*
static void bits_to_bytes()
{
  int l = 0;
  for (int j = 0; j < FRAME_HEIGHT/16; j++) {
    for (int b = 0; b < 16; b++) {
      for (int i = 0; i < FRAME_WIDTH*4; i+=4) {
        int shift = b%8;
        int index_add = b/8 + 2;
        image[l++] = (!!(data[i + index_add + (j*FRAME_WIDTH*4)] & 1<<shift))*255;
      }
    }
  }
}


static void bits_to_bytes_diff_old()
{
  int l = 0;
  for (int j = 0; j < FRAME_HEIGHT/16; j++) {
    for (int b = 0; b < 16; b++) {
      for (int i = 0; i < FRAME_WIDTH*4; i+=4) {
        int shift = b%8;
        int index_add = b/8;
        image[l++] = (!!(data[i + index_add + (j*FRAME_WIDTH*4)] & 1<<shift))*255;
      }
    }
  }
}

*/

static void bits_to_bytes_inter()
{
  int l = 0;

  for (int j = 0; j < FRAME_HEIGHT/16; j++) {
    for (int i = 0; i < FRAME_WIDTH*4; i+=4) {
      *((uint16_t*)(image_data + l)) = *((uint16_t*)(data + i + (j*FRAME_WIDTH*4)));
      l+=2;
    }
  }
}



static void bits_to_bytes_diff()
{
  bits_to_bytes_inter();

  for (int y = 0; y < FRAME_HEIGHT; y++) {
    for (int x = 0; x < FRAME_WIDTH; x++) {
      uint32_t index = ((y/16) * (FRAME_WIDTH*2)) + (x*2) + ((y%16)>7);
      uint8_t bit = image_data[index] & (1 << (y&7));

      image[x + y*FRAME_WIDTH] = (!!bit)*255;
    }
  }

}



#endif /* LOC_ENABLE_SAVE_IMAGE */

void adjust_fps(const double req_interval) 
{
    double current_interval, avg_interval;
    
    // Initialize for 25 FPS
    static double specific_interval = 40.0/1000.0;
    static long delay = 0;  

    clock_gettime(CLOCK_REALTIME, &__gettime_now); 
    __time_difference.tv_sec = __gettime_now.tv_sec - __prev_time.tv_sec; 
    __time_difference.tv_nsec = __gettime_now.tv_nsec - __prev_time.tv_nsec; 
    current_interval = ((__time_difference.tv_sec * 1000000000.0) + __time_difference.tv_nsec)/1000000000.0;
 
    __frames++; 
    if ((__frames % __interval) == 0) {
      __time_difference.tv_sec = __gettime_now.tv_sec - __start_time.tv_sec; 
      __time_difference.tv_nsec = __gettime_now.tv_nsec - __start_time.tv_nsec; 
      avg_interval = ((__time_difference.tv_sec * 1000000000.0) + __time_difference.tv_nsec)/1000000000.0; 

      if (specific_interval > (25.0/1000.0))
        specific_interval += ((req_interval - (avg_interval/__frames))) / 2.0;
      else
        specific_interval = 40.0/1000.0;

      fprintf(stdout, "%s - FPS: %lf, AvgTime: %lf, led_queue_size: %d\r\n",__msg, __frames/avg_interval, 1000.0*(avg_interval/__frames), g_led_dectector.leds_queue_size);
      fflush(stdout);
      __frames = 0; 
      __start_time = __gettime_now; 
    }
    
     
    delay = (long)((specific_interval - current_interval ) * 1000000); 
    
    // If the required delay is less than 1ms, call the usleep(0) with zero sleep value.
    if (delay > 100)
    { 
      //usleep(delay);
    }
    else
    {
      //usleep(0);
    }
    clock_gettime(CLOCK_REALTIME, &__prev_time); 
}
#endif /* LOCALIZATION_DEBUG > 0*/

extern uint32_t led_detected;

static void process_framebuffer(RASPITEX_STATE *raspitex_state)
{
  static uint32_t current_frame = 0;
  double current_time;

  glReadPixels(0,0,FRAME_WIDTH,FRAME_HEIGHT/16, GL_RGBA , GL_UNSIGNED_BYTE, data);

  if (raspitex_state->current_buf)
  {
    current_time = raspitex_state->prev_buff_time;//(raspitex_state->current_buf->pts )/1000.0;
    g_led_dectector.is_new_frame = !!(raspitex_state->current_buf);
    led_detector_process(&g_led_dectector, data, current_time, current_frame++);
#if LOCALIZATION_DEBUG > 0
#ifdef LOC_ENABLE_SAVE_IMAGE
  if (raspitex_state->save_image)
  {
    static int cc = 0;
    char fname[32];
    cc++;
    if (cc > raspitex_state->save_image_warmup && led_detected)
    {
      unsigned error;

      led_detected = 0;

      printf("Saving Image\n");
      sprintf(fname, "%03d.png", cc); 
      bits_to_bytes_diff();
      error = lodepng_encode_file(fname, image, FRAME_WIDTH, FRAME_HEIGHT, LCT_GREY, BITS_PER_BYTE);
      if(error) 
        printf("errorin saving frame: %d\n",error);
      }
  }
#endif
    adjust_fps(40.0/1000.0);
#endif /* LOCALIZATION_DEBUG > 0 */
  }
}


static char * loadshader(const char *filename) {
  FILE* f = fopen(filename, "rb");
  char *src;
  
  if (!f) {
    printf("shader file not found %s\n", filename);
    return 0;
  }

  fseek(f,0,SEEK_END);
  int sz = ftell(f);
  fseek(f,0,SEEK_SET);
  src = (char*)malloc(sz+1);
  fread(src,1,sz,f);
  src[sz] = 0; //null terminate it!
  fclose(f);

  return src;
}


/**
 * Creates the OpenGL ES 2.X context and builds the shaders.
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
int sbpp_init(RASPITEX_STATE *state)
{
  int rc;
  char *format_src;
  char *src;
  
  state->egl_config_attribs = sbpp_egl_config_attribs;
  rc = raspitexutil_gl_init_2_0(state);

  if (rc != 0)
    return rc;

  format_src = loadshader("/usr/share/glsl/binfragshader.glsl.c");
  src = (char*)malloc(strlen(format_src) + 32);
  sprintf(src, format_src, state->luminence_thresh);
  free(format_src);
  sbpp_shader.fragment_source = src;

  rc = raspitexutil_build_shader_program(&sbpp_shader);
  
  GLCHK(glUseProgram(sbpp_shader.program));
  GLCHK(glUniform1i(sbpp_shader.uniform_locations[0], 0)); // tex unit
  GLCHK(glUseProgram(0));

  rc = raspitexutil_build_shader_program(&diff_shader);
  GLCHK(glUseProgram(diff_shader.program));
  GLCHK(glUniform1i(diff_shader.uniform_locations[0], 0)); // tex unit
  GLCHK(glUseProgram(0));


  rc = raspitexutil_build_shader_program(&copy_shader);
  GLCHK(glUseProgram(copy_shader.program));
  GLCHK(glUniform1i(copy_shader.uniform_locations[0], 0)); // tex unit
  GLCHK(glUseProgram(0));

  data = malloc(FRAME_WIDTH*FRAME_HEIGHT/2);
#ifdef LOC_ENABLE_SAVE_IMAGE
  image = malloc(FRAME_WIDTH*FRAME_HEIGHT*4);
  image_data = malloc(FRAME_WIDTH*FRAME_HEIGHT*4);
#endif /* LOC_ENABLE_SAVE_IMAGE */
  
  // Default parameters for ledDetector.
  led_detector_init(&g_led_dectector, state);

  START_FPS("Localizer", 100);

  return rc;
}

extern int bg_counter;

/**
 * Draws a 2x2 grid with each shell showing the entire MMAL buffer from a
 * different EGL image target.
 */
int sbpp_redraw(RASPITEX_STATE *raspitex_state)
{
  static int frame_counter = 0;

  frame_counter++;

  if (bg_counter < 50)
    return 0;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLCHK(glUseProgram(sbpp_shader.program));
      GLCHK(glUniform1i(sbpp_shader.uniform_locations[0], 0)); // tex unit
      GLCHK(glUniform1i(sbpp_shader.uniform_locations[1], 1)); // tex2 unit

      GLCHK(glActiveTexture(GL_TEXTURE0));
      GLCHK(glEnableVertexAttribArray(sbpp_shader.attribute_locations[0]));
      GLCHK(glVertexAttribPointer(sbpp_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
      GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture[0]));
      GLCHK(glActiveTexture(GL_TEXTURE1));
      GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture[1]));
      GLCHK(glActiveTexture(GL_TEXTURE0));
      GLCHK(glVertexAttrib2f(sbpp_shader.attribute_locations[1], -0.5f, 0.5f));
      GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
      GLCHK(glDisableVertexAttribArray(sbpp_shader.attribute_locations[0]));
      GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0));
      GLCHK(glActiveTexture(GL_TEXTURE1));
      GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0));
    GLCHK(glUseProgram(0));


  process_framebuffer(raspitex_state);

  return 0;
}

int sbpp_open(RASPITEX_STATE *state)
{
   state->is_ready = 1;
   return 0;
}
