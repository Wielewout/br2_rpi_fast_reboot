/*
 * Led.c
 *
 *  Created on: Nov 26, 2018
 *      Author: hqss
 */
#include <sys/types.h>
#include <pthread.h>
#include "Configurations.h"
#include "Led.h" 

void led_init_vals(led *l, uint16_t x, uint16_t y, uint16_t one_zero_thresh, uint16_t led_radius, uint16_t frame_number, double frame_time, uint32_t area)
{
  l->x = x;
  l->y = y;
  l->area = area;
  l->id = 0;
  l->buffer_index = 0;
  l->buffer_size = 0;
  l->current_bit_start_time = frame_time;
  l->transmission_start_time = frame_time;
  l->one_zero_thresh = one_zero_thresh;
  l->led_radius = led_radius;
#if DEBUG_LED  
  l->debug_buffer_index = 0;
  l->debug_prev_bit_index = 0;
#endif
  l->start_frame_index = frame_number;
  l->is_first_frame = 1;
  memset(l->buffer, 0, LED_BUFFER_LENGTH);
}

led* led_create_vals(led_detector *ld, uint16_t x, uint16_t y)
{
  led *l = (led*)malloc(sizeof(led));
  led_init_vals(l, x, y, ld->one_zero_thresh, ld->led_radius, ld->frame_number, ld->frame_time, ld->area);
  
  return l;
}

#define GENPOLY 0x0017 /* x^4 + x^2 + x + 1 */

uint32_t led_calculate_checksum(uint32_t val)
{
  int i;

  i=1;
  while (val>=16) { /* >= 2^4, so degree(val) >= degree(genpoly) */
    if (((val >> (16-i)) & 1) == 1)
    val ^= GENPOLY << (12-i); /* reduce with GENPOLY */
    i++;
  }
  return val;
}


typedef struct bit_access_field_t {
  uint8_t a0:1;
  uint8_t a1:1;
  uint8_t a2:1;
  uint8_t a3:1;
  uint8_t a4:1;
  uint8_t a5:1;
  uint8_t a6:1;
  uint8_t a7:1;
} bit_access_field;

uint32_t led_get_roi_sum(led *l, uint8_t *frame, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
  uint32_t sum = 0;
  bit_access_field *sa;

  for (uint32_t i = y1; i < y2 && sum <= l->one_zero_thresh; i+=8)
  {
    for (uint32_t j = x1; j < x2 && sum <= l->one_zero_thresh; j++)
    {
      uint32_t index = (i/16 * (FRAME_WIDTH*2)) + j*2 + (i%16>7);
      sa = (bit_access_field*) (frame + index);
      sum += (sa->a0 + sa->a1 + sa->a2 + sa->a3 + sa->a4 + sa->a5 + sa->a6 + sa->a7);
    }
  }

  return sum;
}

uint8_t led_process(led *l, uint8_t *frame, double frame_time, uint8_t is_new_frame)
{
  uint32_t i;
  uint32_t sum;
  uint32_t x1, y1, x2, y2;
  uint32_t data;
  uint32_t shift;
  uint32_t checksum;
  uint8_t current_frame_bit;
  uint8_t status = 0;

  if (l->id)
    return 1;


  x1 = (l->x > l->led_radius) ? (l->x - l->led_radius) : 0;
  y1 = (l->y > l->led_radius) ? (l->y - l->led_radius) : 0;

  x2 = ((l->x + l->led_radius) < FRAME_WIDTH  ) ? (l->x + l->led_radius) :  (FRAME_WIDTH);
  y2 = ((l->y + l->led_radius) < FRAME_HEIGHT ) ? (l->y + l->led_radius) :  (FRAME_HEIGHT);

  sum = led_get_roi_sum(l, frame, x1, y1, x2, y2);
  
  current_frame_bit = sum > l->one_zero_thresh;
  frame_time = is_new_frame?frame_time: ((frame_time + l->prev_frame_time)/2); 


  uint8_t is_bit_flip = (l->prev_frame_bit != current_frame_bit);
  uint8_t is_end_transmission = ((frame_time - l->current_bit_start_time ) >= (BIT_TRANSFER_TIME*3 ));
  
  
  if ((frame_time - l->transmission_start_time ) >= (BIT_TRANSFER_TIME * (MESSAGE_LENGTH + 2)) ) {
    is_end_transmission = 1;
  }

  if (!l->is_first_frame && (is_bit_flip || is_end_transmission)) 
  {
    double elapsed_time = frame_time - l->current_bit_start_time;
    uint8_t is_data = 0;
    if (current_frame_bit == 1) {
      is_data = elapsed_time > BIT_TRANSFER_TIME/2;
    } else {
      is_data = elapsed_time > (BIT_TRANSFER_TIME/2 + FRAME_TRANSFER_TIME_F);
    }
    if (is_bit_flip && ((l->buffer_index == 0) || is_data)) {
      l->buffer[l->buffer_index] = !current_frame_bit;
      l->buffer_index++;
      l->current_bit_start_time = frame_time;
    } else {
      // ignore
    }
     
#if DEBUG_LED
    l->debug_prev_bit[l->debug_prev_bit_index] = l->prev_frame_bit;
    l->debug_prev_bit_index++;
#endif

  }
  
#if DEBUG_LED
  if (l->debug_buffer_index < (LED_BUFFER_LENGTH*6)) 
  {
    l->debug_buffer[l->debug_buffer_index] = current_frame_bit;
    l->debug_buffer_time[l->debug_buffer_index] = (frame_time - l->current_bit_start_time);//frame_time;
  }
  l->debug_buffer_index++;
#endif
  
  if (is_end_transmission) {
    status = 2;
  }

  if (l->buffer_index >= MESSAGE_LENGTH) {
    i = 1;

    shift = DATA_LENGTH - 1;
    data = 0;
    for (;i < (PREAMBLE_LENGTH + DATA_LENGTH); i++) {
      data |=  l->buffer[i] << shift--;
    }

    shift = CHECKSUM_LENGTH - 1;
    checksum = 0;
    for (;i < (PREAMBLE_LENGTH + DATA_LENGTH + CHECKSUM_LENGTH); i++) {
      checksum |=  l->buffer[i] << shift--;
    }

    if (led_calculate_checksum(data) == checksum) {
      l->id = data;
      status = 1;
    } else {
      status = 2;
    }
  }

#if DEBUG_LED
    if (status != 0 && l->buffer_index > 20) 
    {
      fprintf(stdout, "status: %d, buffer[%d]: (%d, %d)\n", status, l->buffer_index, l->x, l->y);
      //for (i = 0; i < l->buffer_index; i++) {
      //  fprintf(stdout, "%d", l->buffer[i]);
      //}
      fprintf(stdout, "\n");
      fprintf(stdout, "debug_buffer[%d]: ", l->debug_buffer_index); 
      //for (i = 0; i < l->debug_buffer_index; i++) {
      //  fprintf(stdout, "%d", l->debug_buffer[i]); 
      //}
      fprintf(stdout, "\n");
      fflush(stdout);
      /*
      if (status == 2) {
        for (i = 0; i < l->debug_buffer_index; i++) {
          fprintf(stdout, "buffer: %d, time: %f\n", l->debug_buffer[i], l->debug_buffer_time[i] - l->debug_buffer_time[0]);
        }
        fflush(stdout);
      } 
      */
    }
#endif

  l->prev_frame_bit = current_frame_bit;
  l->prev_frame_time = frame_time;
  l->is_first_frame = 0; 

  return status;
}
