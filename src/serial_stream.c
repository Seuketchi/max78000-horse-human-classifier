/**
 * @file    serial_stream.c
 * @brief   Serial streaming utilities implementation for MAX78000 CNN projects.
 */

#include <stdio.h>
#include <string.h>

#include "serial_stream.h"
#include "app_config.h"
#include "mxc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Markers for Python script to detect */
#define IMG_START_MARKER    "<<<IMG_START>>>"
#define IMG_END_MARKER      "<<<IMG_END>>>"
#define RESULT_MARKER       "<<<RESULT>>>"

/*******************************************************************************
 * Code
 ******************************************************************************/

void serial_send_image_start(int width, int height, int capture_id)
{
    printf("\n%s\n", IMG_START_MARKER);
    printf("WIDTH:%d\n", width);
    printf("HEIGHT:%d\n", height);
    printf("CAPTURE_ID:%d\n", capture_id);
    printf("FORMAT:RGB888\n");
    printf("DATA_START\n");
}

void serial_send_image_end(void)
{
    printf("DATA_END\n");
    printf("%s\n\n", IMG_END_MARKER);
}

void serial_print_capture_info(int capture_id, const char *class_name,
                                int confidence, uint32_t inference_time)
{
    printf("\n%s\n", RESULT_MARKER);
    printf("CAPTURE_ID:%d\n", capture_id);
    printf("CLASS:%s\n", class_name);
    printf("CONFIDENCE:%d\n", confidence);
    printf("INFERENCE_TIME_US:%u\n", (unsigned int)inference_time);
    printf("%s\n\n", RESULT_MARKER);
}

void serial_stream_ppm(const uint32_t *cnn_buffer, int width, int height)
{
    uint32_t pixel;
    uint8_t r, g, b;

    if (cnn_buffer == NULL) {
        return;
    }

    /* PPM header (P3 = ASCII, P6 = binary) */
    /* Using P3 (ASCII) for easier serial transmission */
    printf("P3\n");
    printf("# Captured from MAX78000 CNN\n");
    printf("%d %d\n", width, height);
    printf("255\n");

    /* Output pixel data as ASCII RGB values */
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int idx = row * width + col;

            /* CNN buffer format: (B<<16)|(G<<8)|R XOR 0x00808080 */
            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            printf("%d %d %d ", r, g, b);

            /* Add newline every few pixels for readability */
            if ((col + 1) % 8 == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }
}

void serial_stream_hex(const uint32_t *cnn_buffer, int width, int height)
{
    uint32_t pixel;
    uint8_t r, g, b;

    if (cnn_buffer == NULL) {
        return;
    }

    printf("=== HEX IMAGE DATA ===\n");
    printf("Size: %dx%d\n", width, height);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int idx = row * width + col;

            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            printf("%02X%02X%02X", r, g, b);
        }
        printf("\n");
    }
    printf("=== END HEX DATA ===\n");
}

/* Base64 encoding table */
static const char base64_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void serial_stream_base64(const uint32_t *cnn_buffer, int width, int height)
{
    uint32_t pixel;
    uint8_t r, g, b;
    uint8_t triplet[3];
    int triplet_idx = 0;
    int line_len = 0;

    if (cnn_buffer == NULL) {
        return;
    }

    printf("<<<BASE64_IMG_START>>>\n");
    printf("WIDTH:%d,HEIGHT:%d\n", width, height);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int idx = row * width + col;

            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            /* Process R, G, B bytes */
            uint8_t rgb[3] = {r, g, b};
            for (int i = 0; i < 3; i++) {
                triplet[triplet_idx++] = rgb[i];

                if (triplet_idx == 3) {
                    /* Encode 3 bytes to 4 base64 chars */
                    putchar(base64_table[(triplet[0] >> 2) & 0x3F]);
                    putchar(base64_table[((triplet[0] & 0x03) << 4) | ((triplet[1] >> 4) & 0x0F)]);
                    putchar(base64_table[((triplet[1] & 0x0F) << 2) | ((triplet[2] >> 6) & 0x03)]);
                    putchar(base64_table[triplet[2] & 0x3F]);

                    triplet_idx = 0;
                    line_len += 4;

                    /* Add newline every 76 chars for readability */
                    if (line_len >= 76) {
                        putchar('\n');
                        line_len = 0;
                    }
                }
            }
        }
    }

    /* Handle remaining bytes */
    if (triplet_idx > 0) {
        for (int i = triplet_idx; i < 3; i++) {
            triplet[i] = 0;
        }

        putchar(base64_table[(triplet[0] >> 2) & 0x3F]);
        putchar(base64_table[((triplet[0] & 0x03) << 4) | ((triplet[1] >> 4) & 0x0F)]);

        if (triplet_idx > 1) {
            putchar(base64_table[((triplet[1] & 0x0F) << 2) | ((triplet[2] >> 6) & 0x03)]);
        } else {
            putchar('=');
        }
        putchar('=');
    }

    printf("\n<<<BASE64_IMG_END>>>\n");
}
