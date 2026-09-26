#ifndef HEATSTROKE_SERVICE_H
#define HEATSTROKE_SERVICE_H

#pragma once
#include "string.h"
#include "stdio.h"

// BlueNRG Configuration Header
#include "bluenrg_conf.h"

// Core BlueNRG Bluetooth Low Energy (BLE) Stack Headers
#include "bluenrg_def.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_gatt_server.h"
#include "bluenrg_types.h"

/* Define custom 128-bit UUIDs */
#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
    do                                                                                                                                                                   \
    {                                                                                                                                                                    \
        uuid_struct[0] = uuid_0;                                                                                                                                         \
        uuid_struct[1] = uuid_1;                                                                                                                                         \
        uuid_struct[2] = uuid_2;                                                                                                                                         \
        uuid_struct[3] = uuid_3;                                                                                                                                         \
        uuid_struct[4] = uuid_4;                                                                                                                                         \
        uuid_struct[5] = uuid_5;                                                                                                                                         \
        uuid_struct[6] = uuid_6;                                                                                                                                         \
        uuid_struct[7] = uuid_7;                                                                                                                                         \
        uuid_struct[8] = uuid_8;                                                                                                                                         \
        uuid_struct[9] = uuid_9;                                                                                                                                         \
        uuid_struct[10] = uuid_10;                                                                                                                                       \
        uuid_struct[11] = uuid_11;                                                                                                                                       \
        uuid_struct[12] = uuid_12;                                                                                                                                       \
        uuid_struct[13] = uuid_13;                                                                                                                                       \
        uuid_struct[14] = uuid_14;                                                                                                                                       \
        uuid_struct[15] = uuid_15;                                                                                                                                       \
    } while (0)

/* Custom UUIDs for Heatstroke service */
#define COPY_HEATSTROKE_SERVICE_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x01, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_HEATSTROKE_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x02, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_ENV_TEMP_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x03, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_SKIN_TEMP_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x04, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_RH_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x05, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_EXP_TO_SUN_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x06, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)
#define COPY_HEART_RATE_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x07, 0x00, 0xE1, 0x80, 0x02, 0x34, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA, 0xAA)


tBleStatus Add_Heatstroke_Service(void);
tBleStatus Heatstroke_Update(float risk_score);
tBleStatus EnvTemp_Update(float temp);
tBleStatus SkinTemp_Update(float skin_temp);
tBleStatus Humidity_Update(float rh);
tBleStatus ExpToSun_Update(float exp_to_sun);
tBleStatus HeartRate_Update(float heart_rate);

void Read_Request_CB(uint16_t handle);
uint16_t Heatstroke_GetCharHandle(void);
void GAP_DisconnectionComplete_CB(void);
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
void Heatstroke_SetValues(float risk_score,
                          float env_temp,
                          float skin_temp,
                          float rh,
                          float exp_to_sun,
                          float heart_rate);


#endif /* HEATSTROKE_SERVICE_H */
