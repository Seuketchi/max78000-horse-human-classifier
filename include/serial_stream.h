/**
 * @file    serial_stream.h
 * @brief   Serial streaming utilities for MAX78000 CNN projects.
 *          Streams images and results to PC via UART for viewing/capture.
 */

#ifndef SERIAL_STREAM_H_
#define SERIAL_STREAM_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Stream format options */
typedef enum {
    STREAM_FORMAT_RAW,      /**< Raw binary RGB888 data */
    STREAM_FORMAT_PPM,      /**< PPM image format (easy to view/save) */
    STREAM_FORMAT_HEX       /**< Hex dump (for debugging) */
} stream_format_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief   Send image data over serial in PPM format.
 *          PPM can be opened directly by many image viewers and Python.
 *
 * @param   cnn_buffer  CNN input buffer (packed pixels XOR 0x00808080).
 * @param   width       Image width.
 * @param   height      Image height.
 */
void serial_stream_ppm(const uint32_t *cnn_buffer, int width, int height);

/**
 * @brief   Send image data over serial as base64-encoded PNG-style header.
 *          Easier to capture in terminal and decode.
 *
 * @param   cnn_buffer  CNN input buffer.
 * @param   width       Image width.
 * @param   height      Image height.
 */
void serial_stream_base64(const uint32_t *cnn_buffer, int width, int height);

/**
 * @brief   Send image as simple hex dump with markers.
 *
 * @param   cnn_buffer  CNN input buffer.
 * @param   width       Image width.
 * @param   height      Image height.
 */
void serial_stream_hex(const uint32_t *cnn_buffer, int width, int height);

/**
 * @brief   Print capture summary with classification results.
 *
 * @param   capture_id      Capture number/ID.
 * @param   class_name      Predicted class name.
 * @param   confidence      Confidence percentage.
 * @param   inference_time  Inference time in microseconds.
 */
void serial_print_capture_info(int capture_id, const char *class_name,
                                int confidence, uint32_t inference_time);

/**
 * @brief   Send a marker for Python script to detect image start.
 */
void serial_send_image_start(int width, int height, int capture_id);

/**
 * @brief   Send a marker for Python script to detect image end.
 */
void serial_send_image_end(void);

#endif /* SERIAL_STREAM_H_ */
