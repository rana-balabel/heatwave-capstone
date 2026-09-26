#ifndef RTOSTASKS_H
#define RTOSTASKS_H
#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "main.h"

#include "MAX30205.h"
#include "DHT11.h"
#include "LTR390.h"
#include "MAX30100.h"

#include "ai_datatypes_defines.h"
#include "ai_platform.h"
#include "heatwaves_model.h"
#include "heatwaves_model_data.h"
#include "app_bluenrg_ms.h"

extern I2C_HandleTypeDef hi2c1;

void StartMAX30100Task(void *argument);
void StartLTR390Task(void *argument);
void StartSkinTempHeat(void *argument);
void StartAITask(void *argument);

// Task handles
extern osThreadId_t heatSkinTempTaskHandle;
extern osThreadId_t max30100TaskHandle;
extern osThreadId_t uvIndexTaskHandle;
extern osThreadId_t AITaskHandle;

// Task attributes
extern const osThreadAttr_t heatSkinTemp_attributes;
extern const osThreadAttr_t max30100Task_attributes;
extern const osThreadAttr_t uvIndexTask_attributes;
extern const osThreadAttr_t AITaskHandle_attributes;

#define UV_THRESHOLD 3          // Minimum UV index to count as "sun exposure"
#define UV_SAMPLING_INTERVAL 10 // In seconds, matches osDelay(2000)

#endif
