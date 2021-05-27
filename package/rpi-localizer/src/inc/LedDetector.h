/*
 * LedDetector.h
 *
 *  Created on: Nov 26, 2018
 *      Author: hqss
 */

#ifndef LEDDETECTOR_H_
#define LEDDETECTOR_H_

#include <string.h>
#include "Configurations.h"
#include "RaspiTex.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "queue.h"
#include "Led.h"

typedef struct led_detector_t {
  queue_node  *leds;
  uint32_t    leds_queue_size;
  uint8_t     prev_bit_frame[FRAME_HEIGHT * FRAME_WIDTH / 8];
  uint8_t     is_new_frame;
  uint32_t    is_first_frame;
  uint16_t    frame_number;
  double      frame_time;

  uint16_t    minx;
  uint16_t    miny;
  uint16_t    maxx;
  uint16_t    maxy;
  uint32_t    area;
  uint32_t    led_find_radius;
  uint16_t    led_radius;
  uint32_t    led_blob_size;
  uint32_t    one_zero_thresh;
} led_detector;

struct led_t;
typedef struct led_t led;

void        led_detector_init(led_detector *ld, RASPITEX_STATE *state);
void        led_detector_destroy(led_detector *ld);
void        led_detector_detect_leds(led_detector *ld, uint8_t *bFrame);
void        led_detector_check_and_add_led(led_detector *ld, uint16_t x, uint16_t y);
void        led_detector_flood_check(led_detector *ld, uint16_t x, uint16_t y);
uint32_t    led_detector_process(led_detector *ld, uint8_t *bFrame, double frame_time, uint32_t frame_number);
uint8_t     led_detector_add_led(led_detector *ld, led *l);
led*        led_detector_find_led(led_detector *ld, uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif /* LEDDETECTOR_H_ */
