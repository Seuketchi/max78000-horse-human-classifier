/**
 * @file    tft_utils.c
 * @brief   TFT display utilities implementation for MAX78000 CNN projects.
 */

#include <stdio.h>
#include <string.h>

#include "tft_utils.h"
#include "app_config.h"

/* Platform headers */
#include "mxc.h"
#include "tft_ili9341.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static int s_tft_initialized = 0;

/* Font for text display */
#ifdef TFT_ENABLE
extern const unsigned char Arial12x12[];
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

tft_status_t tft_utils_init(void)
{
#ifdef TFT_ENABLE
    int ret;

    printf("Initializing TFT display...\n");

    /* Initialize the TFT display */
    ret = MXC_TFT_Init(NULL, NULL);
    if (ret != E_NO_ERROR) {
        printf("TFT init failed with error %d\n", ret);
        return TFT_STATUS_ERROR;
    }

    /* Set rotation for landscape mode */
    MXC_TFT_SetRotation(ROTATE_270);

    /* Clear screen to black */
    MXC_TFT_ClearScreen();

    s_tft_initialized = 1;
    printf("TFT initialized successfully\n");

    return TFT_STATUS_OK;
#else
    printf("TFT display not enabled in config\n");
    return TFT_STATUS_ERROR;
#endif
}

void tft_utils_display_image(int x, int y, int width, int height,
                              const uint8_t *rgb565_data)
{
#ifdef TFT_ENABLE
    if (!s_tft_initialized || rgb565_data == NULL) {
        return;
    }

    /* Create area structure */
    area_t area = {
        .x = x,
        .y = y,
        .w = width,
        .h = height
    };

    /* Display the image */
    MXC_TFT_WriteBufferRGB565(area.x, area.y, (uint8_t *)rgb565_data, width, height);
#else
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)rgb565_data;
#endif
}

void tft_utils_display_cnn_buffer(int x, int y, int width, int height,
                                   const uint32_t *cnn_buffer)
{
#ifdef TFT_ENABLE
    static uint8_t rgb565_line[TFT_WIDTH * 2];  /* One line buffer */
    uint32_t pixel;
    uint8_t r, g, b;
    uint16_t rgb565;
    int idx;
    int line_idx;

    if (!s_tft_initialized || cnn_buffer == NULL) {
        return;
    }

    /* Convert and display line by line to save memory */
    for (int row = 0; row < height; row++) {
        line_idx = 0;
        for (int col = 0; col < width; col++) {
            idx = row * width + col;

            /* CNN buffer format: (B<<16)|(G<<8)|R XOR 0x00808080 */
            pixel = cnn_buffer[idx] ^ 0x00808080U;
            r = (uint8_t)(pixel & 0xFF);
            g = (uint8_t)((pixel >> 8) & 0xFF);
            b = (uint8_t)((pixel >> 16) & 0xFF);

            /* Convert to RGB565 */
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            /* Store in big-endian format for TFT */
            rgb565_line[line_idx++] = (rgb565 >> 8) & 0xFF;
            rgb565_line[line_idx++] = rgb565 & 0xFF;
        }

        /* Write this line to TFT */
        MXC_TFT_WriteBufferRGB565(x, y + row, rgb565_line, width, 1);
    }
#else
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)cnn_buffer;
#endif
}

void tft_utils_print(int x, int y, const char *text,
                      uint16_t fg_color, uint16_t bg_color)
{
#ifdef TFT_ENABLE
    if (!s_tft_initialized || text == NULL) {
        return;
    }

    MXC_TFT_SetForeGroundColor(fg_color);
    MXC_TFT_SetBackGroundColor(bg_color);
    MXC_TFT_PrintFont(x, y, ARIAL12X12, (char *)text, NULL);
#else
    (void)x;
    (void)y;
    (void)text;
    (void)fg_color;
    (void)bg_color;
#endif
}

void tft_utils_show_results(const char (*class_names)[20],
                             const int *confidences,
                             int num_classes,
                             int predicted_class)
{
#ifdef TFT_ENABLE
    char buf[32];
    int bar_x = 140;
    int bar_y = 180;
    int bar_width = 150;
    int bar_height = 20;
    int spacing = 25;

    if (!s_tft_initialized) {
        return;
    }

    for (int i = 0; i < num_classes; i++) {
        int y_pos = bar_y + (i * spacing);
        int conf = confidences[i];
        int fill_width = (conf * bar_width) / 100;
        uint16_t bar_color = (i == predicted_class) ? TFT_GREEN : TFT_BLUE;

        /* Class name */
        snprintf(buf, sizeof(buf), "%s:", class_names[i]);
        tft_utils_print(10, y_pos + 4, buf, TFT_WHITE, TFT_BLACK);

        /* Background bar */
        tft_utils_fill_rect(bar_x, y_pos, bar_width, bar_height, TFT_BLACK);

        /* Filled portion */
        if (fill_width > 0) {
            tft_utils_fill_rect(bar_x, y_pos, fill_width, bar_height, bar_color);
        }

        /* Border */
        /* Draw manually using lines would be ideal, but simplified here */

        /* Percentage text */
        snprintf(buf, sizeof(buf), "%d%%", conf);
        tft_utils_print(bar_x + bar_width + 5, y_pos + 4, buf, TFT_WHITE, TFT_BLACK);
    }

    /* Show prediction */
    snprintf(buf, sizeof(buf), ">> %s <<", class_names[predicted_class]);
    tft_utils_print(80, bar_y - 30, buf, TFT_YELLOW, TFT_BLACK);
#else
    (void)class_names;
    (void)confidences;
    (void)num_classes;
    (void)predicted_class;
#endif
}

void tft_utils_clear(uint16_t color)
{
#ifdef TFT_ENABLE
    if (!s_tft_initialized) {
        return;
    }

    MXC_TFT_SetBackGroundColor(color);
    MXC_TFT_ClearScreen();
#else
    (void)color;
#endif
}

void tft_utils_fill_rect(int x, int y, int width, int height, uint16_t color)
{
#ifdef TFT_ENABLE
    if (!s_tft_initialized) {
        return;
    }

    MXC_TFT_FillRect(x, y, width, height, color);
#else
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color;
#endif
}
