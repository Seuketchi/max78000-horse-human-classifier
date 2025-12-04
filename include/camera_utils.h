/**
 * @file    camera_utils.h
 * @brief   Camera utilities for MAX78000 CNN projects.
 *          Provides camera initialization, capture, and image processing.
 */

#ifndef CAMERA_UTILS_H_
#define CAMERA_UTILS_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Camera operation status codes */
typedef enum {
    CAM_STATUS_OK = 0,
    CAM_STATUS_ERROR,
    CAM_STATUS_OVERFLOW,
    CAM_STATUS_TIMEOUT
} cam_status_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief   Initialize the camera subsystem.
 *
 * @param   freq        Camera clock frequency in Hz.
 * @param   width       Image width in pixels.
 * @param   height      Image height in pixels.
 * @param   dma_channel DMA channel to use for streaming.
 *
 * @return  CAM_STATUS_OK on success, error code otherwise.
 */
cam_status_t camera_utils_init(uint32_t freq, uint32_t width, uint32_t height, 
                                int dma_channel);

/**
 * @brief   Capture an image and process it for CNN inference.
 *
 * This function captures a frame from the camera and converts it to the
 * format required by the CNN accelerator. The packed pixel format is:
 * (B<<16)|(G<<8)|R, then XOR'd with 0x00808080 to convert to signed range.
 *
 * @param   cnn_buffer      Output buffer for CNN input data (packed pixels).
 * @param   cnn_buffer_size Size of cnn_buffer in 32-bit words.
 * @param   rgb565_buffer   Optional output buffer for RGB565 display data.
 *                          Pass NULL if not needed.
 * @param   rgb565_size     Size of rgb565_buffer in bytes.
 *
 * @return  CAM_STATUS_OK on success, error code otherwise.
 */
cam_status_t camera_utils_capture(uint32_t *cnn_buffer, uint32_t cnn_buffer_size,
                                   uint8_t *rgb565_buffer, uint32_t rgb565_size);

/**
 * @brief   Get the raw image buffer pointer.
 *
 * This can be used for ASCII art or other processing after capture.
 *
 * @param   buffer      Pointer to receive buffer address.
 * @param   length      Pointer to receive buffer length.
 * @param   width       Pointer to receive image width.
 * @param   height      Pointer to receive image height.
 *
 * @return  CAM_STATUS_OK on success, error code otherwise.
 */
cam_status_t camera_utils_get_image(uint8_t **buffer, uint32_t *length,
                                     uint32_t *width, uint32_t *height);

#endif /* CAMERA_UTILS_H_ */
