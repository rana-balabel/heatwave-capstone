#include <stdbool.h>
#include "MAX30100.h"
extern I2C_HandleTypeDef hi2c1;

float meanDiff(float M, meanDiffFilter_t *f) {
    // Remove the old value from the sum
    f->sum -= f->values[f->index];

    // Replace it with the new value
    f->values[f->index] = M;
    f->sum += M;

    // Advance the index
    f->index = (f->index + 1) % MEAN_FILTER_SIZE;

    // Grow count until full
    if (f->count < MEAN_FILTER_SIZE)
        f->count++;

    float avg = f->sum / f->count;
    return avg - M;
}

void lowPassButterworthFilter(float x, butterworthFilter_t *f) {

	const float cf[3] = {0.04125353724172031722, -0.51398189421967566126, 1.34896774525279439239};
    f->v[0] = f->v[1];
    f->v[1] = f->v[2];


    // Fs = 100Hz, Fc = 7.5Hz constants
    // this will allow us to sample from 50bpm to 220 bpm
    f->v[2] = (cf[0] * x) + cf[1] * f->v[0] + cf[2] * f->v[1];;

    f->result = (f->v[0] + f->v[2]) + 2 * f->v[1];
}

void MAX30100_Init() {
	// Set to Heart Rate Mode
	uint8_t mode = 0x02;
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_MODE_CONF, 1, &mode, 1, HAL_I2C_MAX_DELAY);

	// Set IR current to 24mA and Red current to 0mA
	uint8_t current = (0x0 << 4) | 0x07;
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_CURR_CONF, 1, &current, 1, HAL_I2C_MAX_DELAY);

	// Set the Sample rate to 100 HZ i.e. 100 samples per second.
	// At a time, FIFO can read up to 16 samples and gets full, so time taken
	// effectively is 16/100 = 0.16s for FIFO to get full.

	// Set ADC resolution to 15 bits, this has higher accuracy
	// but can lead to more power draw (change later based on power consumption)
	uint8_t spo2_config = 0x01 << 2 | 0x02;
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_SP02_CONF, 1, &spo2_config, 1, HAL_I2C_MAX_DELAY);

	// When starting a new heart-rate conversion, recommended to first clear the FIFO_WR_PTR, OVF_COUNTER, and FIFO_RD_PTR
	// registers to all zeros (0x00) to ensure the FIFO is empty and in a known state.
	uint8_t zero_state = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_WRITE_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_OV_COUNTER_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);
	HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_READ_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);

}
