/**
 * @file    app_config.h
 * @brief   Application configuration header for CNN inference projects.
 *          Customize this file for each new project.
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/*******************************************************************************
 * Image Configuration
 ******************************************************************************/

/** Image width in pixels */
#define IMAGE_SIZE_X        (64 * 2)

/** Image height in pixels */
#define IMAGE_SIZE_Y        (64 * 2)

/** Camera frequency in Hz */
#define CAMERA_FREQ         (5 * 1000 * 1000)

/** RGB565 display buffer size: two bytes per pixel */
#define DATA565_SIZE        (IMAGE_SIZE_X * IMAGE_SIZE_Y * 2)

/** Number of 32-bit words for CNN FIFO input */
#define INPUT_WORDS         16384

/*******************************************************************************
 * Feature Toggles
 ******************************************************************************/

/** Enable TFT display for live video feed */
#define TFT_ENABLE          0

/** Enable serial image streaming to PC (for capture/viewing) */
#define SERIAL_STREAM_ENABLE 1

/** Enable ASCII art preview of captured images (serial console) */
#define ASCII_ART_ENABLE    0

/** ASCII art downscale ratio (higher = smaller output) */
#define ASCII_ART_RATIO     2

/** Live feed mode: continuous capture without button press */
#define LIVE_FEED_ENABLE    1

/** Live feed frame delay in milliseconds (lower = faster, more CPU) */
#define LIVE_FEED_DELAY_MS  50

/** Use sample data instead of camera capture (for testing) */
/* #define USE_SAMPLEDATA */

/*******************************************************************************
 * Hardware Configuration
 ******************************************************************************/

/** Button index for capture trigger (PB1/SW1 = 0) */
#define CAPTURE_BUTTON      0

/** LED for overflow indication */
#define OVERFLOW_LED        LED2

/** LED for status indication */
#define STATUS_LED1         LED1
#define STATUS_LED2         LED2

/*******************************************************************************
 * CNN Configuration - Override if needed
 ******************************************************************************/

/* CNN_NUM_OUTPUTS is defined in cnn.h, but can be overridden here if needed */
/* #define CNN_NUM_OUTPUTS 2 */

#endif /* APP_CONFIG_H_ */
