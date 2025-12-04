/**
 * @file    display_utils.c
 * @brief   Display utilities implementation for MAX78000 CNN projects.
 */

#include <stdio.h>
#include <string.h>

#include "display_utils.h"
#include "app_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Extended brightness ramp for better detail (70 levels, dark -> bright) */
static const char *BRIGHTNESS_EXTENDED = 
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

/* Standard brightness ramp (10 levels) - good balance of detail and speed */
static const char *BRIGHTNESS_STANDARD = "@%#*+=-:. ";

/*******************************************************************************
 * Code
 ******************************************************************************/

void display_ascii_art(const uint8_t *img, int width, int height,
                       int ratio, const char *brightness)
{
    int x, y;
    uint8_t r, g, b;
    uint8_t Y;
    const uint8_t *srcPtr;
    size_t num_chars;
    const char *bstr;
    int char_idx;

    if (img == NULL) {
        return;
    }

    /* Use default brightness if not specified */
    bstr = (brightness != NULL) ? brightness : BRIGHTNESS_STANDARD;
    num_chars = strlen(bstr);

    /* Ensure ratio is at least 1 */
    if (ratio < 1) {
        ratio = 1;
    }

    /* Aspect ratio correction: terminal chars are ~2x taller than wide
     * So we skip 2x more rows than columns */
    int x_step = ratio;
    int y_step = ratio * 2;  /* Compensate for terminal character aspect ratio */

    for (y = 0; y < height; y += y_step) {
        for (x = 0; x < width; x += x_step) {
            /* Calculate position in buffer (4 bytes per pixel: R,G,B,0) */
            srcPtr = img + (y * width + x) * 4;
            
            r = srcPtr[0];
            g = srcPtr[1];
            b = srcPtr[2];

            /* Improved luminance using BT.601 coefficients (fixed-point) 
             * Y = 0.299*R + 0.587*G + 0.114*B 
             * Approximated as: Y = (77*R + 150*G + 29*B) >> 8 */
            Y = (uint8_t)(((uint16_t)(77 * r) + (uint16_t)(150 * g) + (uint16_t)(29 * b)) >> 8);

            /* Map luminance to character index */
            char_idx = (Y * (num_chars - 1)) / 255;
            
            /* Invert: dark pixels = dense chars, bright pixels = sparse chars */
            putchar(bstr[(num_chars - 1) - char_idx]);
        }
        putchar('\n');
    }
}

void display_ascii_art_from_cnn(const uint32_t *cnn_buffer, int width, int height,
                                 int ratio)
{
    int x, y;
    uint8_t r, g, b;
    uint8_t Y;
    uint32_t pixel;
    size_t num_chars;
    const char *bstr = BRIGHTNESS_STANDARD;
    int char_idx;
    int idx;

    if (cnn_buffer == NULL) {
        return;
    }

    num_chars = strlen(bstr);

    /* Ensure ratio is at least 1 */
    if (ratio < 1) {
        ratio = 1;
    }

    /* Aspect ratio correction */
    int x_step = ratio;
    int y_step = ratio * 2;

    for (y = 0; y < height; y += y_step) {
        for (x = 0; x < width; x += x_step) {
            idx = y * width + x;
            
            /* CNN buffer format: (B<<16)|(G<<8)|R XOR 0x00808080 */
            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            /* BT.601 luminance */
            Y = (uint8_t)(((uint16_t)(77 * r) + (uint16_t)(150 * g) + (uint16_t)(29 * b)) >> 8);

            /* Map luminance to character */
            char_idx = (Y * (num_chars - 1)) / 255;
            putchar(bstr[(num_chars - 1) - char_idx]);
        }
        putchar('\n');
    }
}

void display_ascii_art_detailed(const uint32_t *cnn_buffer, int width, int height,
                                 int ratio)
{
    int x, y;
    uint8_t r, g, b;
    uint8_t Y;
    uint32_t pixel;
    size_t num_chars;
    const char *bstr = BRIGHTNESS_EXTENDED;
    int char_idx;
    int idx;

    if (cnn_buffer == NULL) {
        return;
    }

    num_chars = strlen(bstr);

    if (ratio < 1) {
        ratio = 1;
    }

    int x_step = ratio;
    int y_step = ratio * 2;

    for (y = 0; y < height; y += y_step) {
        for (x = 0; x < width; x += x_step) {
            idx = y * width + x;
            
            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            Y = (uint8_t)(((uint16_t)(77 * r) + (uint16_t)(150 * g) + (uint16_t)(29 * b)) >> 8);

            char_idx = (Y * (num_chars - 1)) / 255;
            putchar(bstr[(num_chars - 1) - char_idx]);
        }
        putchar('\n');
    }
}

void display_separator(int width, char ch)
{
    for (int i = 0; i < width; i++) {
        putchar(ch);
    }
    putchar('\n');
}

void display_title(const char *title, int width)
{
    int title_len;
    int padding;

    if (title == NULL) {
        return;
    }

    title_len = (int)strlen(title);
    padding = (width - title_len) / 2;

    if (padding > 0) {
        for (int i = 0; i < padding; i++) {
            putchar(' ');
        }
    }
    printf("%s\n", title);
}
