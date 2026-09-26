#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "main.h"

#define MAX30100_ADDR_WRITE			   		(0x57 << 1)
#define MAX30100_ADDR_READ			   		(0x57 << 1) | 1

#define MAX30100_MODE_CONF  				0x06
#define MAX30100_CURR_CONF					0x09
#define MAX30100_SP02_CONF					0x07

#define MAX30100_FIFO_DATA_REG 				0x05
#define MAX30100_FIFO_WRITE_PTR				0x02
#define MAX30100_FIFO_OV_COUNTER_PTR		0x03
#define MAX30100_FIFO_READ_PTR				0x04

// 100ms delay
#define HAL_I2C_MAX_DELAY  					100

// IR buffer size
// At a time we will take 256 IR samples to compute the BPM
#define IR_BUFFER_SIZE						512
#define FIFO_BUFFER_SIZE					64

#define MEAN_FILTER_SIZE 					15

#define MIN_PEAK_THRESHOLD 					10
#define MAX_PEAKS 							10
#define MIN_PEAK_DISTANCE 					40

#define BPM_COUNT_LIMIT						10

typedef struct {
    float values[MEAN_FILTER_SIZE];
    uint8_t index;
    float sum;
    uint8_t count;
} meanDiffFilter_t;

typedef struct {
    float v[3];     // filter state (delayed outputs)
    float result;   // filtered output
} butterworthFilter_t;

float meanDiff(float M, meanDiffFilter_t *f);
void lowPassButterworthFilter(float x, butterworthFilter_t *f);
void MAX30100_Init();
