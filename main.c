/**
 * @file    main.c
 * @brief   Horse-or-Human CNN Inference Demo for MAX78000
 *
 * This is the main application file that uses the modular utilities.
 * Customize app_config.h and the class definitions below for new projects.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Platform headers */
#include "mxc.h"
#include "mxc_device.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "icc.h"
#include "led.h"
#include "pb.h"
#include "dma.h"

/* CNN headers (auto-generated) */
#include "cnn.h"
#include "weights.h"

/* Application modules */
#include "app_config.h"
#include "camera_utils.h"
#include "inference_utils.h"
#include "display_utils.h"
#ifdef TFT_ENABLE
#include "tft_utils.h"
#endif
#ifdef SERIAL_STREAM_ENABLE
#include "serial_stream.h"
#endif

/*******************************************************************************
 * Definitions - Customize these for your project
 ******************************************************************************/

/** Application name for display */
#define APP_NAME    "Horse-or-Human Demo"

/** Class names - must match CNN_NUM_OUTPUTS */
static const char CLASS_NAMES[CNN_NUM_OUTPUTS][20] = {
    "Horse",
    "Human"
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** RGB565 buffer for TFT display */
static uint8_t data565[DATA565_SIZE];

/** Input buffer for CNN (packed pixels) */
static uint32_t input_buffer[INPUT_WORDS];

/** Capture counter for image naming */
static int capture_count = 0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void system_init(void);
static int hardware_init(void);
static void wait_for_button(const char *message);
static void run_inference_loop(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief   Initialize system clocks and peripherals.
 */
static void system_init(void)
{
#if defined(BOARD_FTHR_REVA)
    /* Wait for PMIC 1.8V to become available (~180 ms after power up) */
    MXC_Delay(200000);

    /* Enable camera power */
    Camera_Power(POWER_ON);

    printf("\n\n%s - Feather Board\n", APP_NAME);
#else
    printf("\n\n%s\n", APP_NAME);
#endif

    MXC_ICC_Enable(MXC_ICC0); /* Enable cache */

    /* Switch to 100 MHz clock */
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();

    printf("Waiting...\n");

    /* Let debugger interrupt if needed */
    MXC_Delay(SEC(2));
}

/**
 * @brief   Initialize hardware peripherals.
 *
 * @return  0 on success, -1 on failure.
 */
static int hardware_init(void)
{
    int dma_channel;
    cam_status_t cam_ret;

    /* DMA initialization */
    MXC_DMA_Init();
    dma_channel = MXC_DMA_AcquireChannel();

    /* Camera initialization */
    cam_ret = camera_utils_init(CAMERA_FREQ, IMAGE_SIZE_X, IMAGE_SIZE_Y, dma_channel);
    if (cam_ret != CAM_STATUS_OK) {
        printf("Camera initialization failed!\n");
        return -1;
    }

#ifdef TFT_ENABLE
    /* TFT display initialization */
    if (tft_utils_init() != TFT_STATUS_OK) {
        printf("TFT initialization failed! Continuing without display.\n");
    }
#endif

    /* CNN initialization */
    if (inference_init() != INFERENCE_OK) {
        printf("CNN initialization failed!\n");
        return -1;
    }

    return 0;
}

/** Live feed mode flag */
static volatile int live_feed_active = 0;

/**
 * @brief   Wait for button press with a message.
 *
 * @param   message     Message to display while waiting.
 */
static void wait_for_button(const char *message)
{
    if (message != NULL) {
        printf("%s\n", message);
    }
    while (!PB_Get(CAPTURE_BUTTON)) {
        /* wait */
    }
}

/**
 * @brief   Check if button was pressed (non-blocking).
 *
 * @return  1 if button pressed, 0 otherwise.
 */
static int check_button_press(void)
{
    return PB_Get(CAPTURE_BUTTON);
}

/**
 * @brief   Clear terminal screen using ANSI escape codes.
 */
static void clear_screen(void)
{
    /* ANSI escape: move cursor to home position */
    printf("\033[H");
}

/**
 * @brief   Run single capture mode.
 */
static void run_single_capture(inference_result_t *result)
{
    cam_status_t cam_ret;
#ifdef TFT_ENABLE
    int confidences[CNN_NUM_OUTPUTS];
#endif

    LED_Off(STATUS_LED1);
    LED_Off(STATUS_LED2);

    capture_count++;
    printf("\n=== Capture #%d ===\n", capture_count);

    /* Capture image from camera */
    cam_ret = camera_utils_capture(input_buffer, INPUT_WORDS, data565, DATA565_SIZE);
    if (cam_ret == CAM_STATUS_OVERFLOW) {
        printf("Camera overflow! Halting.\n");
        while (1) {
            /* halt */
        }
    }

#ifdef TFT_ENABLE
    /* Display camera image on TFT */
    tft_utils_display_cnn_buffer(0, 0, IMAGE_SIZE_X, IMAGE_SIZE_Y, input_buffer);
#endif

    /* Start CNN and load input data */
    inference_start();
    inference_load_input(input_buffer, INPUT_WORDS);

    /* Wait for inference to complete */
    if (inference_wait(result) != INFERENCE_OK) {
        printf("Inference failed!\n");
        return;
    }

    printf("\n*** PASS ***\n\n");

    /* Print classification results */
    inference_print_results(result, CLASS_NAMES, CNN_NUM_OUTPUTS);

    /* Show prediction */
    printf("Prediction: %s (%d%% confidence)\n\n", 
           CLASS_NAMES[result->predicted_class], 
           result->confidence_percent);

#ifdef SERIAL_STREAM_ENABLE
    /* Send result info for Python script */
    serial_print_capture_info(capture_count, 
                              CLASS_NAMES[result->predicted_class],
                              result->confidence_percent,
                              result->inference_time_us);

    /* Stream the image to PC */
    printf("Streaming image to PC...\n");
    serial_send_image_start(IMAGE_SIZE_X, IMAGE_SIZE_Y, capture_count);
    serial_stream_ppm(input_buffer, IMAGE_SIZE_X, IMAGE_SIZE_Y);
    serial_send_image_end();
    printf("Image sent! Use Python script to capture.\n");
#endif

#ifdef TFT_ENABLE
    /* Display results on TFT */
    for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
        confidences[i] = (1000 * result->softmax[i] + 0x4000) >> 15;
        confidences[i] = confidences[i] / 10;
    }
    tft_utils_show_results(CLASS_NAMES, confidences, CNN_NUM_OUTPUTS, result->predicted_class);
#endif

#if ASCII_ART_ENABLE
    /* Display ASCII art preview */
    display_ascii_art_from_cnn(input_buffer, IMAGE_SIZE_X, IMAGE_SIZE_Y, 
                                ASCII_ART_RATIO);
#endif
}

#if LIVE_FEED_ENABLE
/**
 * @brief   Run live feed mode with continuous capture.
 */
static void run_live_feed(void)
{
    inference_result_t result;
    cam_status_t cam_ret;
    int frame_count = 0;
#ifdef TFT_ENABLE
    int confidences[CNN_NUM_OUTPUTS];
    char buf[32];
#endif

    printf("\n=== LIVE FEED MODE ===\n");
    printf("Press PB1 (SW1) to exit live feed\n\n");
    MXC_Delay(MXC_DELAY_SEC(1));

#ifdef TFT_ENABLE
    /* Clear TFT screen */
    tft_utils_clear(TFT_BLACK);
#else
    /* Clear terminal screen */
    printf("\033[2J");  /* ANSI clear screen */
#endif
    
    while (1) {
        /* Check for button press to exit */
        if (check_button_press()) {
            printf("\n\nExiting live feed mode...\n");
            MXC_Delay(MXC_DELAY_MSEC(500));  /* Debounce */
            break;
        }

        /* Capture image from camera */
        cam_ret = camera_utils_capture(input_buffer, INPUT_WORDS, data565, DATA565_SIZE);
        if (cam_ret == CAM_STATUS_OVERFLOW) {
            printf("Camera overflow!\n");
            continue;
        }

#ifdef TFT_ENABLE
        /* Display live camera feed on TFT */
        tft_utils_display_cnn_buffer(0, 0, IMAGE_SIZE_X, IMAGE_SIZE_Y, input_buffer);
#endif

        /* Start CNN and load input data */
        inference_start();
        inference_load_input(input_buffer, INPUT_WORDS);

        /* Wait for inference to complete */
        if (inference_wait(&result) != INFERENCE_OK) {
            continue;
        }

        frame_count++;

#ifdef TFT_ENABLE
        /* Display results on TFT below the image */
        for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
            confidences[i] = (1000 * result.softmax[i] + 0x4000) >> 15;
            confidences[i] = confidences[i] / 10;
        }
        
        /* Show frame count */
        snprintf(buf, sizeof(buf), "Frame: %d", frame_count);
        tft_utils_print(140, 10, buf, TFT_WHITE, TFT_BLACK);
        
        /* Show prediction with highlight */
        snprintf(buf, sizeof(buf), ">> %s: %d%% <<", 
                 CLASS_NAMES[result.predicted_class], 
                 result.confidence_percent);
        tft_utils_print(140, 40, buf, TFT_YELLOW, TFT_BLACK);
        
        /* Show all class confidences */
        for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
            uint16_t color = (i == result.predicted_class) ? TFT_GREEN : TFT_WHITE;
            snprintf(buf, sizeof(buf), "%s: %d%%  ", CLASS_NAMES[i], confidences[i]);
            tft_utils_print(140, 70 + (i * 20), buf, color, TFT_BLACK);
        }
#else
        /* Move cursor to top-left for console display */
        clear_screen();

        /* Show frame info and prediction */
        printf("[LIVE] Frame: %d | %s (%d%%)\n", 
               frame_count,
               CLASS_NAMES[result.predicted_class], 
               result.confidence_percent);
        
        /* Show confidence bar */
        printf("[");
        for (int i = 0; i < 20; i++) {
            if (i < result.confidence_percent / 5) {
                putchar('#');
            } else {
                putchar('-');
            }
        }
        printf("] ");
        
        /* Show which class with indicator */
        for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
            int conf = (1000 * result.softmax[i] + 0x4000) >> 15;
            conf = conf / 10;
            if (i == result.predicted_class) {
                printf(">>%s:%d%% ", CLASS_NAMES[i], conf);
            } else {
                printf("  %s:%d%% ", CLASS_NAMES[i], conf);
            }
        }
        printf("\n\n");

#if ASCII_ART_ENABLE
        /* Display ASCII art preview */
        display_ascii_art_from_cnn(input_buffer, IMAGE_SIZE_X, IMAGE_SIZE_Y, 
                                    ASCII_ART_RATIO);
#endif

        printf("\n[Press PB1 to exit live feed]");
#endif

        /* Small delay between frames */
        MXC_Delay(MXC_DELAY_MSEC(LIVE_FEED_DELAY_MS));
    }
}
#endif /* LIVE_FEED_ENABLE */

/**
 * @brief   Main inference loop.
 */
static void run_inference_loop(void)
{
    inference_result_t result;

    /* Enable CNN clock for continuous operation */
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    while (1) {
#if LIVE_FEED_ENABLE
        printf("\n=== MODE SELECT ===\n");
        printf("Press PB1 (SW1) briefly for SINGLE CAPTURE\n");
        printf("Hold PB1 (SW1) for 1 sec for LIVE FEED\n\n");
        
        /* Wait for button press */
        while (!check_button_press()) {
            /* wait */
        }
        
        /* Check if held for live feed mode */
        int hold_count = 0;
        while (check_button_press() && hold_count < 10) {
            MXC_Delay(MXC_DELAY_MSEC(100));
            hold_count++;
        }
        
        if (hold_count >= 10) {
            /* Long press - live feed mode */
            run_live_feed();
        } else {
            /* Short press - single capture */
            run_single_capture(&result);
        }
#else
        /* Single capture mode only */
        wait_for_button("********** Press PB1 (SW1) to capture an image **********");
        run_single_capture(&result);
#endif
    }
}

/**
 * @brief   Application entry point.
 */
int main(void)
{
    /* Initialize system */
    system_init();

    printf("\n*** CNN Inference Test: %s ***\n", APP_NAME);

    /* Initialize hardware */
    if (hardware_init() != 0) {
        printf("Hardware initialization failed! Halting.\n");
        while (1) {
            /* halt */
        }
    }

    /* Run main inference loop */
    run_inference_loop();

    /* Should never reach here */
    return 0;
}

/*
SUMMARY OF OPS
Hardware: 51,368,960 ops (50,432,000 macc; 936,960 comp; 0 add; 0 mul; 0 bitwise)
    Layer 0: 7,340,032 ops (7,077,888 macc; 262,144 comp; 0 add; 0 mul; 0 bitwise)
    Layer 1: 19,267,584 ops (18,874,368 macc; 393,216 comp; 0 add; 0 mul; 0 bitwise)
    Layer 2: 19,070,976 ops (18,874,368 macc; 196,608 comp; 0 add; 0 mul; 0 bitwise)
    Layer 3: 4,792,320 ops (4,718,592 macc; 73,728 comp; 0 add; 0 mul; 0 bitwise)
    Layer 4: 600,064 ops (589,824 macc; 10,240 comp; 0 add; 0 mul; 0 bitwise)
    Layer 5: 295,936 ops (294,912 macc; 1,024 comp; 0 add; 0 mul; 0 bitwise)
    Layer 6: 2,048 ops (2,048 macc; 0 comp; 0 add; 0 mul; 0 bitwise)

RESOURCE USAGE
Weight memory: 57,776 bytes out of 442,368 bytes total (13.1%)
Bias memory:   2 bytes out of 2,048 bytes total (0.1%)
*/
