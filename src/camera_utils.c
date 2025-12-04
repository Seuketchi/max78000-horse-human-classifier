/**
 * @file    camera_utils.c
 * @brief   Camera utilities implementation for MAX78000 CNN projects.
 */

#include <stdio.h>
#include <string.h>

#include "camera_utils.h"
#include "app_config.h"

/* Platform headers */
#include "mxc.h"
#include "camera.h"
#include "led.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t s_image_width  = 0;
static uint32_t s_image_height = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

cam_status_t camera_utils_init(uint32_t freq, uint32_t width, uint32_t height,
                                int dma_channel)
{
    int ret;

    s_image_width  = width;
    s_image_height = height;

    printf("Init Camera.\n");
    camera_init(freq);

    ret = camera_setup(width, height, PIXFORMAT_RGB888,
                       FIFO_THREE_BYTE, STREAMING_DMA, dma_channel);
    if (ret != STATUS_OK) {
        printf("Error returned from setting up camera. Error %d\n", ret);
        return CAM_STATUS_ERROR;
    }

    /* Prevent streaming overflow by setting camera clock prescaler */
    camera_write_reg(0x11, 0x00);

    return CAM_STATUS_OK;
}

cam_status_t camera_utils_capture(uint32_t *cnn_buffer, uint32_t cnn_buffer_size,
                                   uint8_t *rgb565_buffer, uint32_t rgb565_size)
{
    uint8_t *raw;
    uint32_t imgLen;
    uint32_t w, h;
    int cnt = 0;
    uint8_t r, g, b;
    uint16_t rgb;
    int j = 0;
    uint8_t *data = NULL;
    stream_stat_t *stat;

    camera_start_capture_image();

    /* Get image pointer/length/width/height from camera driver */
    camera_get_image(&raw, &imgLen, &w, &h);
    printf("W:%d H:%d L:%d\n", (int)w, (int)h, (int)imgLen);

    /* Read image streaming buffers line by line */
    for (int row = 0; row < (int)h; row++) {
        /* Wait until camera streaming buffer is available */
        while ((data = get_camera_stream_buffer()) == NULL) {
            if (camera_is_image_rcv()) {
                break;
            }
        }

        j = 0;
        /*
         * Data format from camera is assumed to be 4 bytes per pixel: 0x00 B G R
         * (k increments by 4)
         */
        for (int k = 0; k < (int)(4 * w); k += 4) {
            r = data[k];
            g = data[k + 1];
            b = data[k + 2];

            /* Check buffer bounds */
            if ((uint32_t)cnt < cnn_buffer_size) {
                /* Store packed pixel for CNN as (B<<16)|(G<<8)|R then XOR to convert to signed range */
                cnn_buffer[cnt] = ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
                cnn_buffer[cnt] ^= 0x00808080U;
                cnt++;
            }

            /* Convert to RGB565 for display if buffer provided */
            if (rgb565_buffer != NULL && (uint32_t)(j + 1) < rgb565_size) {
                rgb = (uint16_t)(((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3));
                rgb565_buffer[j]     = (uint8_t)((rgb >> 8) & 0xFF);
                rgb565_buffer[j + 1] = (uint8_t)(rgb & 0xFF);
                j += 2;
            }
        }

        /* Release the stream buffer back to camera driver */
        release_camera_stream_buffer();
    }

    /* Check streaming stats for overflow */
    stat = get_camera_stream_statistic();
    if (stat->overflow_count > 0) {
        printf("OVERFLOW DISP = %d\n", stat->overflow_count);
#ifdef OVERFLOW_LED
        LED_On(OVERFLOW_LED);
#endif
        return CAM_STATUS_OVERFLOW;
    }

    return CAM_STATUS_OK;
}

cam_status_t camera_utils_get_image(uint8_t **buffer, uint32_t *length,
                                     uint32_t *width, uint32_t *height)
{
    if (buffer == NULL || length == NULL || width == NULL || height == NULL) {
        return CAM_STATUS_ERROR;
    }

    camera_get_image(buffer, length, width, height);

    return CAM_STATUS_OK;
}
