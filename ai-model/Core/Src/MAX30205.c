#include <stdbool.h>
#include "MAX30205.h"
extern I2C_HandleTypeDef hi2c1;
int Read_SkinTemperature(void)
{
  uint8_t reg = 0x00;
  uint8_t buf[2];

  if (HAL_I2C_Master_Transmit(&hi2c1, MAX30205_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
    return 0;

  if (HAL_I2C_Master_Receive(&hi2c1, MAX30205_ADDR, buf, 2, HAL_MAX_DELAY) != HAL_OK)
    return 0;

  uint8_t msb = buf[0];
  uint8_t lsb = buf[1];
  int16_t raw = (int16_t)((msb << 8) | lsb);
  return (raw / 256.0);
}

uint8_t Read_Config(void)
{
  uint8_t reg = 0x01;
  uint8_t config = 0xFF;

  if (HAL_I2C_Master_Transmit(&hi2c1, MAX30205_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
    return config;

  if (HAL_I2C_Master_Receive(&hi2c1, MAX30205_ADDR, &config, 1, HAL_MAX_DELAY) != HAL_OK)
    return config;

  return config;
}

uint8_t Set_Config(void)
{
  uint8_t reg = 0x01;
  uint8_t config = 0x00;


  // Step 3: Write updated config back to CONFIG register
  uint8_t data[2] = {reg, 0};
  if (HAL_I2C_Master_Transmit(&hi2c1, MAX30205_ADDR, data, 2, HAL_MAX_DELAY) != HAL_OK)
    return 0xFF;

  return config;  // return the new config for confirmation
}
