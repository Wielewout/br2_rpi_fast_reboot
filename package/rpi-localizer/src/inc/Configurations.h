
#ifndef CONFIGURATIONS_H

#define CONFIGURATIONS_H

#include <stdint.h>

#define PREAMBLE_LENGTH           1
#define DATA_LENGTH               16
#define CHECKSUM_LENGTH           4


#define LOCALIZATION_DEBUG 1

#if LOCALIZATION_DEBUG > 0
#define FRAMES_PER_BIT           6
#define LED_RADIUS               50
#define LED_FIND_RADIUS          75
#define LED_ONE_ZERO_THRESHOLD   100
#define LED_BLOB_SIZE            1500
#define LUMINENCE_THRESH         0.1
//#define LOC_ENABLE_SAVE_IMAGE    1
#else
#define FRAMES_PER_BIT           5
#define LED_RADIUS               5
#define LED_FIND_RADIUS          10
#define LED_ONE_ZERO_THRESHOLD   10
#define LED_BLOB_SIZE            150
#define LUMINENCE_THRESH         0.1
#endif

#define MONOCHROME_THRESHOLD      64
#define LED_DATA_MASK             0x7FFF
#define LED_DATA_MSB              (1 << 15)
#define MAX_PREAMBLE_ERROR        5
#define MIN_TIME_SHIFT            -90
#define MAX_TIME_SHIFT            90
#define BIT_TRANSFER_TIME         160
#define FRAME_TRANSFER_TIME       40
#define FRAME_TRANSFER_TIME_F     40.0
#define MESSAGE_MARGIN_TIME       10

#define LED_BUFFER_LENGTH         ((PREAMBLE_LENGTH + DATA_LENGTH + CHECKSUM_LENGTH) * 3)
#define MESSAGE_LENGTH            (PREAMBLE_LENGTH + DATA_LENGTH + CHECKSUM_LENGTH)
#define TIME_SHIFT_JUMP           10

#define FRAME_WIDTH               1920
#define FRAME_HEIGHT              1088

#define VERSION 2

#endif /* CONFIGURATIONS_H */
