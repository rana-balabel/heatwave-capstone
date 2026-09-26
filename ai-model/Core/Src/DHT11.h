#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "main.h"

#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_4
#define DEFAULT_DELAY 500
#define TIMEOUT_DURATION_MS 1000


uint8_t DHT11_ReadByte();
void DHT11_Start (void);
uint8_t DHT11_Read (uint8_t* data);
float DHT11_ComputeHeatIndex(float T, float R);
