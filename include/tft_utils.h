/**
 * @file    tft_utils.h
 * @brief   TFT display utilities for MAX78000 CNN projects.
 *          Provides TFT initialization and image display functions.
 */

#ifndef TFT_UTILS_H_
#define TFT_UTILS_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** TFT display dimensions (ILI9341) */
#define TFT_WIDTH           320
#define TFT_HEIGHT          240

/** Colors in RGB565 format */
#define TFT_BLACK           0x0000
#define TFT_WHITE           0xFFFF
#define TFT_RED             0xF800
#define TFT_GREEN           0x07E0
#define TFT_BLUE            0x001F
#define TFT_YELLOW          0xFFE0
#define TFT_CYAN            0x07FF
#define TFT_MAGENTA         0xF81F

/** TFT operation status codes */
typedef enum {
    TFT_STATUS_OK = 0,
    TFT_STATUS_ERROR
} tft_status_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief   Initialize the TFT display.
 *
 * @return  TFT_STATUS_OK on success, error code otherwise.
 */
tft_status_t tft_utils_init(void);

/**
 * @brief   Display an RGB565 image on the TFT.
 *
 * @param   x           X position on screen.
 * @param   y           Y position on screen.
 * @param   width       Image width.
 * @param   height      Image height.
 * @param   rgb565_data RGB565 pixel data buffer.
 */
void tft_utils_display_image(int x, int y, int width, int height, 
                              const uint8_t *rgb565_data);

/**
 * @brief   Display CNN buffer on TFT (converts from CNN format to RGB565).
 *
 * @param   x           X position on screen.
 * @param   y           Y position on screen.
 * @param   width       Image width.
 * @param   height      Image height.
 * @param   cnn_buffer  CNN input buffer (packed pixels XOR 0x00808080).
 */
void tft_utils_display_cnn_buffer(int x, int y, int width, int height,
                                   const uint32_t *cnn_buffer);

/**
 * @brief   Display a text string on the TFT.
 *
 * @param   x           X position.
 * @param   y           Y position.
 * @param   text        Text string to display.
 * @param   fg_color    Foreground color (RGB565).
 * @param   bg_color    Background color (RGB565).
 */
void tft_utils_print(int x, int y, const char *text, 
                      uint16_t fg_color, uint16_t bg_color);

/**
 * @brief   Display classification result with bar graph.
 *
 * @param   class_names     Array of class name strings.
 * @param   confidences     Array of confidence percentages.
 * @param   num_classes     Number of classes.
 * @param   predicted_class Index of predicted class.
 */
void tft_utils_show_results(const char (*class_names)[20],
                             const int *confidences,
                             int num_classes,
                             int predicted_class);

/**
 * @brief   Clear the TFT screen.
 *
 * @param   color       Fill color (RGB565).
 */
void tft_utils_clear(uint16_t color);

/**
 * @brief   Draw a filled rectangle.
 *
 * @param   x           X position.
 * @param   y           Y position.
 * @param   width       Rectangle width.
 * @param   height      Rectangle height.
 * @param   color       Fill color (RGB565).
 */
void tft_utils_fill_rect(int x, int y, int width, int height, uint16_t color);

#endif /* TFT_UTILS_H_ */
