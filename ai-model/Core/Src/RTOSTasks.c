#include <stdbool.h>
#include "RTOSTasks.h"

/* Definitions for heatSkinTemp task */
osThreadId_t heatSkinTempTaskHandle;
const osThreadAttr_t heatSkinTemp_attributes = {
	.name = "heatSkinTemp",
	.stack_size = 2048,
	.priority = (osPriority_t)osPriorityNormal,
};

/* USER CODE BEGIN PV */
osThreadId_t max30100TaskHandle;
const osThreadAttr_t max30100Task_attributes = {
	.name = "max30100Task",
	.stack_size = 512 * 12,
	.priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t uvIndexTaskHandle;
const osThreadAttr_t uvIndexTask_attributes = {
	.name = "uvIndexTask",
	.stack_size = 2048,
	.priority = (osPriority_t)osPriorityBelowNormal,
};

osThreadId_t AITaskHandle;
const osThreadAttr_t AITaskHandle_attributes = {
	.name = "AITask",
	.stack_size = 2048, // TODO: not sure rn ...
	.priority = (osPriority_t)osPriorityAboveNormal,
};

#define FLAG_SENSOR_START 0x0001U

uint32_t gSunExposureSeconds;
bool gHeartRateReady;
float gAvgBpm;
float gBodyTemperature;
float gTemperature;
float gHumidity;
float gHeatstrokeRiskScore;
uint32_t gRiskLevel;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf(">>> STACK OVERFLOW in task: %s <<<\r\n", pcTaskName);
	while (1)
		; // trap or reset
}
void StartMAX30100Task(void *argument)
{
	// take 10 bpm samples and compute average
	// Static buffers to avoid stack overflow with large IR_BUFFER_SIZE
	static int32_t irData[IR_BUFFER_SIZE];
	static uint8_t fifoData[FIFO_BUFFER_SIZE];

	int ir_index = 0;
	int bpm_sum = 0;
	int measured_bpm_count = 0;
	int total_bpm_tries = 0;
	uint8_t zero_state = 0x00;
	while (1)
	{
		osThreadFlagsWait(FLAG_SENSOR_START, osFlagsWaitAny, osWaitForever);
		printf("Heart rate woke up\r\n");
		gHeartRateReady = 0;
		while (total_bpm_tries < BPM_COUNT_LIMIT)
		{
			while (ir_index < IR_BUFFER_SIZE)
			{
				// wait for FIFO to get full which takes 160ms
				osDelay(160);
				uint8_t fifo_reg = MAX30100_FIFO_DATA_REG;
				HAL_I2C_Master_Transmit(&hi2c1, MAX30100_ADDR_WRITE, &fifo_reg, 1, HAL_MAX_DELAY);
				HAL_I2C_Master_Receive(&hi2c1, MAX30100_ADDR_READ, fifoData, 64, HAL_MAX_DELAY);

				// we only need the first 2 bytes (IR), and ignore the next 2 bytes (RED)
				for (int i = 0; i < FIFO_BUFFER_SIZE; i = i + 4)
				{
					// since its 15 bit resolution MSB first, we right shift by 1
					if (ir_index < IR_BUFFER_SIZE)
					{
						irData[ir_index++] = (((uint16_t)fifoData[i] << 8) | fifoData[i + 1]) >> 1;
					}
				}
				HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_WRITE_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);
				HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_OV_COUNTER_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);
				HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDR_WRITE, MAX30100_FIFO_READ_PTR, 1, &zero_state, 1, HAL_I2C_MAX_DELAY);
			}
			ir_index = 0;

			// DC removal using first-order high-pass filter (moving average)
			// use equations
			static float dcFiltered[IR_BUFFER_SIZE];

			float avg = 0;
			for (int i = 0; i < IR_BUFFER_SIZE; i++)
				avg += irData[i];
			avg /= IR_BUFFER_SIZE;

			for (int i = 0; i < IR_BUFFER_SIZE; i++)
			{
				dcFiltered[i] = irData[i] - avg;
			}

			// Mean median filtering
			static float meanMedianFiltered[IR_BUFFER_SIZE];
			meanDiffFilter_t filter = {0};

			for (int i = 0; i < IR_BUFFER_SIZE; i++)
			{
				meanMedianFiltered[i] = meanDiff(dcFiltered[i], &filter);
			}

			// Removes frequencies higher than a cutoff (Fc = 10hz)
			// and keep the signal as flat and smooth as possible in the passband
			// Fs = 100hz, Fc = 10hz, normalized frequency ratio = 10/100 = 0.1

			static float butterworthFiltered[IR_BUFFER_SIZE];
			butterworthFilter_t bwfilter = {0};

			for (int i = 0; i < IR_BUFFER_SIZE; i++)
			{
				lowPassButterworthFilter(meanMedianFiltered[i], &bwfilter);
				butterworthFiltered[i] = bwfilter.result;
			}

			int peak_indices[MAX_PEAKS] = {0};
			int peak_count = 0;

			// Calculate dynamic threshold based on signal amplitude
			float max_val = butterworthFiltered[0];
			float min_val = butterworthFiltered[0];
			for (int i = 1; i < IR_BUFFER_SIZE; i++)
			{
				if (butterworthFiltered[i] > max_val)
					max_val = butterworthFiltered[i];
				if (butterworthFiltered[i] < min_val)
					min_val = butterworthFiltered[i];
			}
			float dynamic_threshold = max_val * 0.5f;
			if (dynamic_threshold < MIN_PEAK_THRESHOLD)
			{
				dynamic_threshold = MIN_PEAK_THRESHOLD;
			}

			printf("dynamic threshold: %.1f, max: %.1f, min: %.1f\r\n", dynamic_threshold, max_val, min_val);

			// Detect peaks
			for (int i = 1; i < IR_BUFFER_SIZE - 1; i++)
			{
				if (butterworthFiltered[i] > butterworthFiltered[i - 1] &&
					butterworthFiltered[i] > butterworthFiltered[i + 1] &&
					butterworthFiltered[i] > dynamic_threshold)
				{

					if (peak_count == 0 || ((i - peak_indices[peak_count - 1]) > MIN_PEAK_DISTANCE))
					{
						if (peak_count < MAX_PEAKS)
						{
							peak_indices[peak_count++] = i;
						}
					}
				}
			}
			printf("Peaks found: %d\r\n", peak_count);

			// Compute average interval
			if (peak_count >= 2)
			{
				float interval_sum = 0;

				for (int i = 1; i < peak_count; i++)
				{
					interval_sum += (peak_indices[i] - peak_indices[i - 1]);
				}
				// for N peaks we have N-1 intervals
				float avg_interval = interval_sum / (peak_count - 1);

				// 3. Convert to BPM
				float bpm = 60.0f * 100.0f / avg_interval; // 100 Hz sample rate

				// Outlier rejection: only accept BPM in valid physiological range
				if (bpm >= 50.0f && bpm <= 200.0f)
				{
					measured_bpm_count++;
					bpm_sum += bpm;
					gHeartRateReady = 1;
					printf("BPM reading: %.1f \r\n", bpm);
				}
				else
				{
					printf("BPM out of range: %.1f - discarding\r\n", bpm);
				}
			}
			else
			{
				printf("Not enough peaks\r\n");
				osDelay(10);
			}
			total_bpm_tries++;
		}

		// Use debugger to look at value
		if (gHeartRateReady)
		{
			float avg_bpm = bpm_sum / measured_bpm_count;
			printf(" bpm over %d readings is %f: \r\n ", measured_bpm_count, avg_bpm);
			gAvgBpm = avg_bpm;
		}
		measured_bpm_count = 0;
		total_bpm_tries = 0;
		bpm_sum = 0;
		osDelay(2000); // allow some time before next computation
	}
}

void StartLTR390Task(void *argument)
{

	const int num_samples = 20;
	while (1)
	{
		float uv_sum = 0.0f;
		for (int i = 0; i < num_samples; i++)
		{
			UDOUBLE UV_count = LTR390_UVS();
			float uv_index = LTR390Calculate_UVI(UV_count);
			uv_sum += uv_index;
			osDelay(200); // Small delay between samples
		}

		float avg_uv_index = uv_sum / num_samples;
		printf("Avg UV = %.2f\r\n", avg_uv_index);

		if (avg_uv_index >= UV_THRESHOLD)
		{
			gSunExposureSeconds += UV_SAMPLING_INTERVAL;
			printf("In sun: +%ds | Total: %lus\n",
				   UV_SAMPLING_INTERVAL,
				   gSunExposureSeconds);
		}

		osDelay(UV_SAMPLING_INTERVAL * 1000); // Delay before next sampling cycle
	}
}
void StartAITask(void *argument)
{
	gRiskLevel = 3;
	// time period for AI task to sleep after computations
	uint32_t variable_period_ms = gRiskLevel * 60 * 1000;
	// time period to needed to for sensors to do their sampling
	const uint32_t sensor_window_ms = 45000; // 45 seconds
	LTR390_Init();
	MAX30100_Init();

	AI_ALIGNED(4)
	float ai_input_data[AI_HEATWAVES_MODEL_IN_1_SIZE];
	AI_ALIGNED(4)
	float ai_output_data[AI_HEATWAVES_MODEL_OUT_1_SIZE];

	// Chunk of memory used to hold intermediate values for neural network
	AI_ALIGNED(4)
	ai_u8 activations[AI_HEATWAVES_MODEL_DATA_ACTIVATIONS_SIZE];

	const ai_handle act_addr[] = {activations};

	// Pointer to the model
	ai_handle network_model = AI_HANDLE_NULL;

	// Initialize wrapper structs that hold pointers to data and info about the
	// data (tensor height, width, channels)
	ai_buffer *ai_input;
	ai_buffer *ai_output;

	// Create and initialize network
	ai_error err;
	err = ai_heatwaves_model_create_and_init(&network_model, act_addr, NULL);
	if (err.type != AI_ERROR_NONE)
	{
		Error_Handler();
	}

	ai_input = ai_heatwaves_model_inputs_get(network_model, NULL);
	ai_output = ai_heatwaves_model_outputs_get(network_model, NULL);

	ai_input[0].data = AI_HANDLE_PTR(ai_input_data);
	ai_output[0].data = AI_HANDLE_PTR(ai_output_data);

	// Initialize BLE module
	IDB05A2_BlueNRG_MS_Init();

	for (;;)
	{
		osThreadFlagsSet(heatSkinTempTaskHandle, FLAG_SENSOR_START);
		osThreadFlagsSet(max30100TaskHandle, FLAG_SENSOR_START);
		// sleep to give time to for sensors to update
		uint32_t t1 = HAL_GetTick();
		while ((HAL_GetTick() - t1) < sensor_window_ms)
		{
			hci_user_evt_proc();
			osDelay(10);
		}

		// StandardScaler parameters from training (mean and std per feature:
		// env_temp, body_temp, humidity, sun_exposure_min, heart_rate)
		static const float scaler_mean[5] = {34.22461616f, 38.12566433f, 0.41169463f, 57.20161768f, 103.23146749f};
		static const float scaler_std[5] = {10.46643728f, 1.51432449f, 0.15723341f, 36.58061737f, 25.05174793f};

		// Normalize inputs before inference (apply StandardScaler: (x - mean) / std)
		ai_input_data[0] = (gTemperature - scaler_mean[0]) / scaler_std[0];
		ai_input_data[1] = (gBodyTemperature - scaler_mean[1]) / scaler_std[1];
		ai_input_data[2] = (gHumidity / 100.0f - scaler_mean[2]) / scaler_std[2];
		ai_input_data[3] = ((float)gSunExposureSeconds / 60.0f - scaler_mean[3]) / scaler_std[3];
		ai_input_data[4] = (gAvgBpm - scaler_mean[4]) / scaler_std[4];

		// Run inference
		ai_i32 batch;
		batch = ai_heatwaves_model_run(network_model, ai_input, ai_output);
		if (batch != 1)
		{
			Error_Handler();
		}

		gHeatstrokeRiskScore = (ai_output_data)[0];

		// debug signal
		if (gHeatstrokeRiskScore <= 0.50)
		{
			gRiskLevel = 1;
		}
		else if (gHeatstrokeRiskScore > 0.75)
		{
			// change to 1 for testing
			// change to 3 for dev
			gRiskLevel = 1;
		}
		else
		{
			// change to 1 for testing
			// change to 2 for dev
			gRiskLevel = 1;
		}

		variable_period_ms = gRiskLevel * 60 * 1000;

		printf("\n*** Final Report ***\r\n");
		printf("Heart Rate: %.2f BPM\r\n", gAvgBpm);
		printf("Body Temp: %.2f C\r\n", gBodyTemperature);
		printf("Environment Temp: %.2f C\r\n", gTemperature);
		printf("Humidity: %.2f %%\r\n", gHumidity);
		printf("Sun Exposure: %lu seconds\r\n", gSunExposureSeconds);
		printf("heatstroke Risk Score: %.2f\r\n", gHeatstrokeRiskScore);
		printf("********************\r\n\n");

		// Transmit the information via bluetooth
		MX_BlueNRG_MS_Process(gHeatstrokeRiskScore, gTemperature, gBodyTemperature, gHumidity,
							  gSunExposureSeconds, gAvgBpm);
		uint32_t extra_wait = 0;
		if (variable_period_ms > sensor_window_ms)
			// instead of sleeping the AI task, keep sending BLE events to keep the BLE stack alive
			extra_wait = variable_period_ms - sensor_window_ms;

		uint32_t t0 = HAL_GetTick();
		while ((HAL_GetTick() - t0) < extra_wait)
		{
			hci_user_evt_proc();
			osDelay(10);
		}
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartSkinTempHeat(void *argument)
{
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	UBYTE heat_index_data[5] = {0};

	// temp accumulators for averaging
	float tempC = 0.0f;
	float humidity = 0.0f;

	const int num_samples = 15;

	for (;;)
	{
		osThreadFlagsWait(FLAG_SENSOR_START, osFlagsWaitAny, osWaitForever);
		printf("Skin temp and heat index woke up\r\n");
		tempC = 0.0f;
		humidity = 0.0f;

		// Sample DHT11
		if (DHT11_Read((uint8_t *)&heat_index_data) == 0)
		{
			humidity = (float)heat_index_data[0] + (float)heat_index_data[1] / 10.0f;
			tempC = (float)heat_index_data[2] + (float)heat_index_data[3] / 10.0f;
			tempC -= 4;
		}
		else
		{
			printf("Failed to read DHT11");
		}

		float avg_skin_temp = 0.0f;
		for (int i = 0; i < num_samples; i++)
		{
			UBYTE sample = Read_SkinTemperature();
			avg_skin_temp += (float)sample;
			osDelay(100);
		}

		// compute averages
		avg_skin_temp /= num_samples;

		// Print and update global variables
		printf("Avg Skin Temperature: %.1f C\r\n", avg_skin_temp);
		printf("Avg Temp: %.1f °C  Avg Humidity: %.1f %% \r\n", tempC, humidity);

		gTemperature = tempC;
		gHumidity = humidity;
		// offsets to add based on correlation with ambient temperature
		if (gTemperature < 30.0)
		{
			gBodyTemperature = avg_skin_temp + 3.0;
			if (gBodyTemperature < 36)
			{
				gBodyTemperature = 36.5;
			}
		}
		else
		{
			// board gets too hot in the sun, so don't need to add offset here
			gBodyTemperature = avg_skin_temp;
		}
		printf("Body Temperature: %.1f C\r\n", gBodyTemperature);

		osDelay(2000);
	}

	/* USER CODE END 5 */
}
