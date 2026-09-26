#include <stdint.h>
#include "main.h"

#define MAX30205_ADDR (0x48 << 1)

int Read_SkinTemperature(void);
uint8_t Read_Config(void);
