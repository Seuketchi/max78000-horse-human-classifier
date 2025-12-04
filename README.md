# MAX78000 CNN Inference Project - Modular Template

This project demonstrates a modular architecture for MAX78000 CNN inference applications with camera input.

## Features

- **Live Camera Capture** - Capture images from camera and run CNN inference
- **Serial Image Streaming** - Stream images to your laptop for viewing/saving
- **TFT Display Support** - Optional live video display on ILI9341 TFT
- **ASCII Art Preview** - Console-based image preview
- **Modular Design** - Easy to reuse for other CNN projects

## Quick Start

### 1. Build and Flash
```bash
# In CFS Terminal
make -r clean
make -r
openocd -s C:/MaximSDK/Tools/OpenOCD/scripts -f interface/cmsis-dap.cfg -f target/max78000.cfg -c "program build/max78000.elf reset exit"
```

### 2. Capture Images on Your Laptop

Install Python dependencies:
```bash
pip install pyserial pillow
```

Run the capture script (find your COM port in Device Manager):
```bash
python tools/capture_images.py --port COM3
```

Then press PB1 on the MAX78000 to capture images. They will be saved to the `captures/` folder.

## Project Structure

```
project/
├── main.c                  # Application entry point (customize per project)
├── cnn.c                   # Auto-generated CNN code
├── cnn.h                   # Auto-generated CNN header
├── weights.h               # Auto-generated weights
├── softmax.c               # Softmax implementation
├── Makefile                # Build system (don't modify)
├── project.mk              # Project-specific build config
├── include/
│   ├── app_config.h        # Application configuration (customize per project)
│   ├── camera_utils.h      # Camera utilities header
│   ├── inference_utils.h   # Inference utilities header
│   └── display_utils.h     # Display utilities header
└── src/
    ├── camera_utils.c      # Camera capture and processing
    ├── inference_utils.c   # CNN inference wrapper
    └── display_utils.c     # ASCII art and display functions
```

## Reusing for a New Project

### 1. Copy the Reusable Modules

Copy these files to your new project:
- `include/camera_utils.h` and `src/camera_utils.c`
- `include/inference_utils.h` and `src/inference_utils.c`
- `include/display_utils.h` and `src/display_utils.c`

### 2. Create Your `app_config.h`

Copy `include/app_config.h` and modify:

```c
/* Image size - match your CNN input */
#define IMAGE_SIZE_X        (64 * 2)   // 128 pixels
#define IMAGE_SIZE_Y        (64 * 2)   // 128 pixels

/* Camera frequency */
#define CAMERA_FREQ         (5 * 1000 * 1000)

/* CNN input buffer size in 32-bit words */
#define INPUT_WORDS         16384

/* Feature toggles */
#define ASCII_ART_ENABLE    1          // Set to 0 to disable
#define ASCII_ART_RATIO     2          // Downscale ratio
```

### 3. Update Your `main.c`

Customize the application name and class labels:

```c
#define APP_NAME    "My-New-Classifier Demo"

static const char CLASS_NAMES[CNN_NUM_OUTPUTS][20] = {
    "Class A",
    "Class B",
    "Class C"
    // Add more as needed (must match CNN_NUM_OUTPUTS)
};
```

### 4. Update `project.mk`

Ensure these lines are in your `project.mk`:

```makefile
# Include additional source directories
VPATH += src

# Include additional header directories
IPATH += include
```

### 5. Generate CNN Files

Use `ai8xize.py` to generate the CNN-specific files:
- `cnn.c` / `cnn.h`
- `weights.h`
- `softmax.c`

## Module API Reference

### Camera Utils

```c
// Initialize camera
cam_status_t camera_utils_init(uint32_t freq, uint32_t width, uint32_t height, 
                                int dma_channel);

// Capture image for CNN
cam_status_t camera_utils_capture(uint32_t *cnn_buffer, uint32_t cnn_buffer_size,
                                   uint8_t *rgb565_buffer, uint32_t rgb565_size);
```

### Inference Utils

```c
// Initialize CNN engine
inference_status_t inference_init(void);

// Load input data
void inference_load_input(const uint32_t *input_data, uint32_t num_words);

// Start inference (non-blocking)
void inference_start(void);

// Wait for completion and get results
inference_status_t inference_wait(inference_result_t *result);

// Print results to console
void inference_print_results(const inference_result_t *result,
                             const char (*class_names)[20],
                             int num_classes);
```

### Display Utils

```c
// Render CNN buffer as ASCII art
void display_ascii_art_from_cnn(const uint32_t *cnn_buffer, int width, int height,
                                 int ratio);
```

## Building

```bash
make clean
make
```

## Flashing

Use the VS Code tasks or OpenOCD directly.

## License

Based on Maxim Integrated example code. See license headers in source files.
