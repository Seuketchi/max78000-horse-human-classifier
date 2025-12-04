/**
 * @file    display_utils.h
 * @brief   Display utilities for MAX78000 CNN projects.
 *          Provides ASCII art rendering and other display functions.
 */

#ifndef DISPLAY_UTILS_H_
#define DISPLAY_UTILS_H_

#include <stdint.h>

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief   Render an image as ASCII art to the console.
 *
 * @param   img         Pointer to image data in RGB888 format (R,G,B,0 per pixel).
 * @param   width       Image width in pixels.
 * @param   height      Image height in pixels.
 * @param   ratio       Downscale ratio (1 = full size, 2 = half, etc.).
 * @param   brightness  ASCII brightness string (dark->bright characters).
 *                      Pass NULL to use default "@%#*+=-:. ".
 */
void display_ascii_art(const uint8_t *img, int width, int height, 
                       int ratio, const char *brightness);

/**
 * @brief   Render a packed CNN buffer as ASCII art (standard detail).
 *
 * The CNN buffer format is 32-bit words with (B<<16)|(G<<8)|R XOR 0x00808080.
 * Uses 10-level brightness ramp with aspect ratio correction.
 *
 * @param   cnn_buffer  Pointer to CNN input buffer.
 * @param   width       Image width in pixels.
 * @param   height      Image height in pixels.
 * @param   ratio       Downscale ratio.
 */
void display_ascii_art_from_cnn(const uint32_t *cnn_buffer, int width, int height,
                                 int ratio);

/**
 * @brief   Render a packed CNN buffer as ASCII art (high detail).
 *
 * Uses 70-level brightness ramp for maximum detail.
 * Best for larger terminal windows.
 *
 * @param   cnn_buffer  Pointer to CNN input buffer.
 * @param   width       Image width in pixels.
 * @param   height      Image height in pixels.
 * @param   ratio       Downscale ratio.
 */
void display_ascii_art_detailed(const uint32_t *cnn_buffer, int width, int height,
                                 int ratio);

/**
 * @brief   Print a horizontal line separator.
 *
 * @param   width       Width in characters.
 * @param   ch          Character to use (e.g., '-' or '=').
 */
void display_separator(int width, char ch);

/**
 * @brief   Print a centered title.
 *
 * @param   title       Title string to print.
 * @param   width       Total width for centering.
 */
void display_title(const char *title, int width);

#endif /* DISPLAY_UTILS_H_ */
