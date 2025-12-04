/**
 * @file    inference_utils.c
 * @brief   CNN inference utilities implementation for MAX78000 projects.
 */

#include <stdio.h>
#include <string.h>

/* Platform headers - must come before cnn.h */
#include "mxc.h"

#include "inference_utils.h"
#include "cnn.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Define cnn_time here - declared extern in cnn.h, used by CNN ISR */
volatile uint32_t cnn_time;

/*******************************************************************************
 * Code
 ******************************************************************************/

inference_status_t inference_init(void)
{
    /* Enable peripheral, enable CNN interrupt, turn on CNN clock
     * CNN clock: APB (50 MHz) div 1 */
    cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK, MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);

    cnn_init();          /* Bring state machine into consistent state */
    cnn_load_weights();  /* Load kernels */
    cnn_load_bias();     /* Load biases */
    cnn_configure();     /* Configure state machine */

    return INFERENCE_OK;
}

void inference_load_input(const uint32_t *input_data, uint32_t num_words)
{
    uint32_t i;
    const uint32_t *in = input_data;

    for (i = 0; i < num_words; i++) {
        /* Wait for FIFO not full */
        while (((*((volatile uint32_t *)0x50000004) & 1)) != 0) {
            /* spin */
        }
        /* Write to CNN FIFO register */
        *((volatile uint32_t *)0x50000008) = *in++;
    }
}

inference_status_t inference_run(inference_result_t *result)
{
    if (result == NULL) {
        return INFERENCE_ERROR;
    }

    /* Start CNN processing */
    inference_start();

    /* Wait for completion and get results */
    return inference_wait(result);
}

void inference_start(void)
{
    /* Reset timer before starting */
    cnn_time = 0;

    /* Start CNN processing */
    cnn_start();
}

inference_status_t inference_wait(inference_result_t *result)
{
    int i;
    int max_idx = 0;
    q15_t max_val = 0;

    if (result == NULL) {
        return INFERENCE_ERROR;
    }

    /* Wait for CNN to finish (cnn_time set by ISR) */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; /* Ensure SLEEPDEEP=0 */
    while (cnn_time == 0) {
        __WFI();
    }

    /* Capture inference time */
    result->inference_time_us = cnn_time;

    /* Unload CNN output */
    cnn_unload((uint32_t *)result->raw_output);

    /* Compute softmax */
    softmax_q17p14_q15((const q31_t *)result->raw_output, CNN_NUM_OUTPUTS, result->softmax);

    /* Find the class with highest probability */
    max_val = result->softmax[0];
    max_idx = 0;
    for (i = 1; i < CNN_NUM_OUTPUTS; i++) {
        if (result->softmax[i] > max_val) {
            max_val = result->softmax[i];
            max_idx = i;
        }
    }

    result->predicted_class = max_idx;

    /* Convert Q15 softmax to percentage (0-100) */
    /* Q15 max is 32767 = 100%, so multiply by 100 and divide by 32768 */
    result->confidence_percent = (max_val * 100) >> 15;

    return INFERENCE_OK;
}

void inference_disable(void)
{
    cnn_disable();
}

inference_status_t inference_enable(void)
{
    /* Re-enable CNN clock */
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    return INFERENCE_OK;
}

void inference_print_results(const inference_result_t *result,
                             const char (*class_names)[20],
                             int num_classes)
{
    int i;
    int digs, tens;

    if (result == NULL || class_names == NULL) {
        return;
    }

    printf("Classification results:\n");
    for (i = 0; i < num_classes && i < CNN_NUM_OUTPUTS; i++) {
        digs = (1000 * result->softmax[i] + 0x4000) >> 15;
        tens = digs % 10;
        digs = digs / 10;

        printf("[%7d] -> %20s: %d.%d%%\r\n", 
               (int)result->raw_output[i], class_names[i], digs, tens);
    }
    printf("\n");

#ifdef CNN_INFERENCE_TIMER
    printf("Approximate inference time: %u us\n\n", result->inference_time_us);
#endif
}
