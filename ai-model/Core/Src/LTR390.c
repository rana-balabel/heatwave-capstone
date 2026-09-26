#include "LTR390.h"

extern I2C_HandleTypeDef hi2c1;

/******************************************************************************
  function:	Send one byte of data to  I2C dev
  parameter:
            Addr: Register address
            Value: Write to the value of the register
  Info:
******************************************************************************/
static void LTR390_Write(UBYTE cmd, UBYTE val)
{
  UBYTE Buf[1] = {0};
  Buf[0] = val;
  HAL_I2C_Mem_Write(&hi2c1, LTR390_ADDR, cmd, I2C_MEMADD_SIZE_8BIT, Buf, 1, 0x20);
}

/******************************************************************************
  function:	 read one byte of data to  I2C dev
  parameter:
            Addr: Register address
  Info:
******************************************************************************/
static UBYTE LTR390_Read(UBYTE cmd)
{
  UBYTE Buf[1] = {0};
  HAL_I2C_Mem_Read(&hi2c1, LTR390_ADDR, cmd, I2C_MEMADD_SIZE_8BIT, Buf, 1, 0x20);
  return Buf[0];
}

/******************************************************************************
  function:	TSL2591 Initialization
  parameter:
  Info:
******************************************************************************/
UBYTE LTR390_Init(void)
{

  //    printf("LTR390 VOC Sensor Init\r\n");

  // ID
  UBYTE Rdata = LTR390_Read(0x06);
  if (Rdata != 0xb2)
  {
    printf("Expected ID = 0xB2, but got = 0x%02X\r\n", Rdata);
    return 1;
  }

  LTR390_Write(LTR390_MEAS_RATE, RESOLUTION_20BIT_TIME400MS | RATE_500MS); // default
  LTR390_Write(LTR390_GAIN, GAIN_18);                                      // default

  return 0;
}

float LTR390Calculate_UVI(uint32_t uv_count)
{
  float sensitivity = 2300.0;
  return (uv_count / sensitivity);
}
UDOUBLE LTR390_UVS(void)
{
  LTR390_Write(LTR390_INT_CFG, 0x34);   // UVS_INT_EN=1, Command=0x34
  LTR390_Write(LTR390_MAIN_CTRL, 0x0A); //  UVS in Active Mode
  UDOUBLE Data1 = LTR390_Read(LTR390_UVSDATA);
  UDOUBLE Data2 = LTR390_Read(LTR390_UVSDATA + 1);
  UDOUBLE Data3 = LTR390_Read(LTR390_UVSDATA + 2);
  UDOUBLE uv;
  uv = (Data3 << 16) | (Data2 << 8) | Data1;
  return uv;
}

UDOUBLE LTR390_ALS(void)
{
  LTR390_Write(LTR390_INT_CFG, 0x34);   // UVS_INT_EN=1, Command=0x34
  LTR390_Write(LTR390_MAIN_CTRL, 0x0A); //  UVS in Active Mode
  UDOUBLE Data1 = LTR390_Read(LTR390_UVSDATA);
  UDOUBLE Data2 = LTR390_Read(LTR390_UVSDATA + 1);
  UDOUBLE Data3 = LTR390_Read(LTR390_UVSDATA + 2);
  UDOUBLE als;
  als = (Data3 << 16) | (Data2 << 8) | Data1;
  return als;
}

void LTR390_SetIntVal(UDOUBLE low, UDOUBLE high) // LTR390_THRESH_UP and LTR390_THRESH_LOW
{
  LTR390_Write(0x21, high & 0xff);
  LTR390_Write(0x22, (high >> 8) & 0xff);
  LTR390_Write(0x23, (high >> 16) & 0x0f);
  LTR390_Write(0x24, low & 0xff);
  LTR390_Write(0x25, (low >> 8) & 0xff);
  LTR390_Write(0x26, (low >> 16) & 0x0f);
}
