/*
 * DRIVER FOR DHT11
 * DataSheet Link: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
 */

#include <stdbool.h>
#include "DHT11.h"
uint8_t DHT11_CheckResponse(void);

float DHT11_ComputeHeatIndex(float T, float R)
{
    // Coefficients from the equation
    float c1 = -8.78469475556;
    float c2 = 1.61139411;
    float c3 = 2.33854883889;
    float c4 = -0.14611605;
    float c5 = -0.012308094;
    float c6 = -0.0164248277778;
    float c7 = 0.002211732;
    float c8 = 0.00072546;
    float c9 = -0.000003582;

    float T2 = T * T;
    float R2 = R * R;

    float HI = c1 +
               c2 * T +
               c3 * R +
               c4 * T * R +
               c5 * T2 +
               c6 * R2 +
               c7 * T2 * R +
               c8 * T * R2 +
               c9 * T2 * R2;

    return HI;
}

void DHT11_Start (void)
{
  GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
  GPIO_InitStructPrivate.Pin = DHT11_PIN;
  GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as output
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 0);   // pull the pin low
  HAL_Delay(20);   // wait for 20ms
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);   // pull the pin high
  microDelay(40);   // wait for 30us
  GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as input
}
uint8_t DHT11_CheckResponse(void)
{
    microDelay(40);
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        microDelay(80);
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            microDelay(80); // DHT is about to send data
            return 1;
        }
    }
    return 0;
}

uint8_t DHT11_Read(uint8_t* data)
{
    HAL_Delay(DEFAULT_DELAY);
    DHT11_Start();

    if (!DHT11_CheckResponse()) {
        printf("No response from DHT\n\r");
        return 1;
    }

    for (int i = 0; i < 5; i++)
        data[i] = DHT11_ReadByte();

    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if (data[4] == (sum & 0xFF)) {
//        printf("Checksum OK: %d == %d\r\n", data[4], sum & 0xFF);
        return 0;
    } else {
//        printf("Checksum Error: %d != %d\r\n", data[4], sum & 0xFF);
        return 1;
    }
}


uint8_t DHT11_ReadByte()
{
  uint8_t value = 0;

  for (int i = 0; i < 8; i++)
  {
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET);
    microDelay(30);// check if its high after more than 28us
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
      value |= (1 << (7 - i));
    }
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET);
  }
  return value;
}
