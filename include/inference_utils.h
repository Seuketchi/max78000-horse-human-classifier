/**
 * @file    inference_utils.h
 * @brief   CNN inference utilities for MAX78000 projects.
 *          Provides a reusable wrapper around CNN accelerator operations.
 */

#ifndef INFERENCE_UTILS_H_
#define INFERENCE_UTILS_H_

#include <stdint.h>
#include "mxc.h"
#include "cnn.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Inference operation status codes */
typedef enum {
    INFERENCE_OK = 0,
    INFERENCE_ERROR,
    INFERENCE_TIMEOUT
} inference_status_t;

/** Inference result structure */
typedef struct {
    int32_t  raw_output[CNN_NUM_OUTPUTS];   /**< Raw CNN output values */
    q15_t    softmax[CNN_NUM_OUTPUTS];      /**< Softmax probabilities (Q15) */
    int      predicted_class;                /**< Index of highest probability class */
    int      confidence_percent;             /**< Confidence as percentage (0-100) */
    uint32_t inference_time_us;              /**< Inference time in microseconds */
} inference_result_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief   Initialize the CNN inference engine.
 *
 * This function enables the CNN peripheral, loads weights and biases,
 * and configures the state machine.
 *
 * @return  INFERENCE_OK on success, error code otherwise.
 */
inference_status_t inference_init(void);

/**
 * @brief   Load input data into the CNN FIFO.
 *
 * @param   input_data      Pointer to input data buffer (packed pixels).
 * @param   num_words       Number of 32-bit words to load.
 */
void inference_load_input(const uint32_t *input_data, uint32_t num_words);

/**
 * @brief   Run CNN inference and wait for completion.
 *
 * This function starts the CNN, waits for it to complete, and unloads
 * the results. The inference timer value is captured automatically.
 *
 * @param   result          Pointer to result structure to fill.
 *
 * @return  INFERENCE_OK on success, error code otherwise.
 */
inference_status_t inference_run(inference_result_t *result);

/**
 * @brief   Start CNN inference (non-blocking).
 *
 * Use this with inference_wait() for more control over timing.
 */
void inference_start(void);

/**
 * @brief   Wait for CNN inference to complete.
 *
 * @param   result          Pointer to result structure to fill.
 *
 * @return  INFERENCE_OK on success, error code otherwise.
 */
inference_status_t inference_wait(inference_result_t *result);

/**
 * @brief   Disable the CNN peripheral.
 *
 * Call this when inference is no longer needed to save power.
 */
void inference_disable(void);

/**
 * @brief   Re-enable the CNN peripheral after disable.
 *
 * This re-initializes the CNN for another inference session.
 *
 * @return  INFERENCE_OK on success, error code otherwise.
 */
inference_status_t inference_enable(void);

/**
 * @brief   Print classification results to console.
 *
 * @param   result          Pointer to inference result.
 * @param   class_names     Array of class name strings.
 * @param   num_classes     Number of classes.
 */
void inference_print_results(const inference_result_t *result,
                             const char (*class_names)[20],
                             int num_classes);

#endif /* INFERENCE_UTILS_H_ */
