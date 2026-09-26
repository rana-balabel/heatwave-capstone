#include "heatstroke_service.h"
#include <string.h>

uint16_t heatstrokeServHandle;
uint16_t riskCharHandle;
uint16_t envTempCharHandle;
uint16_t skinTempCharHandle;
uint16_t rHCharHandle;
uint16_t expToSunCharHandle;
uint16_t heartRateCharHandle;

typedef struct
{
    float risk_score;
    float env_temp;
    float skin_temp;
    float rh;
    float exp_to_sun;
    float heart_rate;
} HeatstrokeValues_t;

static HeatstrokeValues_t s_vals = {0};

uint8_t set_connectable = TRUE;
uint16_t connection_handle = 0;
uint32_t connected = FALSE;

void Heatstroke_SetValues(float risk_score,
                          float env_temp,
                          float skin_temp,
                          float rh,
                          float exp_to_sun,
                          float heart_rate)
{
    s_vals.risk_score = risk_score;
    s_vals.env_temp   = env_temp;
    s_vals.skin_temp  = skin_temp;
    s_vals.rh         = rh;
    s_vals.exp_to_sun = exp_to_sun;
    s_vals.heart_rate = heart_rate;
}


/**
 * @brief  Add the Heatstroke BLE Service and its characteristic to gatt db,
 * the ble module will extract from gattdb and sends to the app when app wants to read.
 * @retval tBleStatus BLE status
 */
tBleStatus Add_Heatstroke_Service(void)
{
    tBleStatus ret;
    uint8_t uuid[16];

    /* Create custom 128-bit UUIDs */
    COPY_HEATSTROKE_SERVICE_UUID(uuid);
    ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 50, &heatstrokeServHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding Heatstroke Service: 0x%02X\r\n", ret);
        return ret;
    }

    /* Add a characteristic for the heatstroke risk score (float = 4 bytes) */
    COPY_HEATSTROKE_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4, // size in bytes
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &riskCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
     {
         printf("Error adding Heatstroke Risk characteristic: 0x%02X\r\n", ret);
         return ret;
     }

    // Add characteristic: environment temperature (float = 4 bytes) */
    COPY_ENV_TEMP_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &envTempCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding EnvTemp characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    COPY_SKIN_TEMP_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &skinTempCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding SkinTemp characteristic: 0x%02X\r\n", ret);
        return ret;
    }


    COPY_RH_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &rHCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding RH characteristic: 0x%02X\r\n", ret);
        return ret;
    }


    COPY_EXP_TO_SUN_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &expToSunCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding ExpToSun characteristic: 0x%02X\r\n", ret);
        return ret;
    }


    COPY_HEART_RATE_CHAR_UUID(uuid);
    ret = aci_gatt_add_char(heatstrokeServHandle, UUID_TYPE_128, uuid,
                            4,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                            16, 0, &heartRateCharHandle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error adding HeartRate characteristic: 0x%02X\r\n", ret);
        return ret;
    }



    printf("Heatstroke Service & Characteristic added successfully.\r\n");
    return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update the heatstroke risk characteristic value from AI model
 * This will be sent to the app.
 * @param  risk_score  float value representing heatstroke risk.
 * @retval tBleStatus BLE status
 */
tBleStatus Heatstroke_Update(float risk_score)
{
    uint8_t buff[4];
    // needs to be little endian format because arm is little endian
    memcpy(buff, &risk_score, sizeof(float));

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, riskCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating Heatstroke characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}

/**
 * @brief Update environment temperature characteristic (float)
 */
tBleStatus EnvTemp_Update(float temp)
{
    uint8_t buff[4];
    memcpy(buff, &temp, sizeof(float)); // ARM little-endian

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, envTempCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating EnvTemp characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}


tBleStatus SkinTemp_Update(float skinTemp)
{
    uint8_t buff[4];
    memcpy(buff, &skinTemp, sizeof(float)); // ARM little-endian

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, skinTempCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating skinTemp characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}

tBleStatus Humidity_Update(float rh)
{
    uint8_t buff[4];
    memcpy(buff, &rh, sizeof(float)); // ARM little-endian

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, rHCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating RH characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}

tBleStatus ExpToSun_Update(float expToSun)
{
    uint8_t buff[4];
    memcpy(buff, &expToSun, sizeof(float)); // ARM little-endian

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, expToSunCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating ExpToSun characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}

tBleStatus HeartRate_Update(float heartRate)
{
    uint8_t buff[4];
    memcpy(buff, &heartRate, sizeof(float)); // ARM little-endian

    tBleStatus ret = aci_gatt_update_char_value(heatstrokeServHandle, heartRateCharHandle,
                                                0, sizeof(buff), buff);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error updating HeartRate characteristic: 0x%02X\r\n", ret);
        return ret;
    }

    return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Internal helper to get the attribute handle for read requests.
 * @retval uint16_t characteristic handle
 */
uint16_t Heatstroke_GetCharHandle(void)
{
    return riskCharHandle;
}

/**
 * @brief Called when the phone reads one of our characteristics.
 * handle  The attribute handle being read.
 */
void Read_Request_CB(uint16_t handle)
{
    if (handle == riskCharHandle + 1)
    {
        Heatstroke_Update(s_vals.risk_score);
    }
    else if (handle == envTempCharHandle + 1)
    {
        EnvTemp_Update(s_vals.env_temp);
    }
    else if (handle == skinTempCharHandle + 1)
    {
        SkinTemp_Update(s_vals.skin_temp);
    }
    else if (handle == rHCharHandle + 1)
    {
        Humidity_Update(s_vals.rh);
    }
    else if (handle == expToSunCharHandle + 1)
    {
        ExpToSun_Update(s_vals.exp_to_sun);
    }
    else if (handle == heartRateCharHandle + 1)
    {
        HeartRate_Update(s_vals.heart_rate);
    }

    if (connection_handle != 0)
    {
        tBleStatus ret = aci_gatt_allow_read(connection_handle);
        if (ret != BLE_STATUS_SUCCESS)
        {
            PRINTF("aci_gatt_allow_read() failed: 0x%02x\r\n", ret);
        }
    }
}


/**
 * @brief  This function is called when the peer device gets disconnected.
 * @param  None
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
    connected = FALSE;
    PRINTF("Disconnected\n");
    /* Make the device connectable again. */
    set_connectable = TRUE;
}

/**
 * @brief  This function is called when there is a BLE Connection Complete event.
 * @param  uint8_t Address of peer device
 * @param  uint16_t Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{
    connected = TRUE;
    connection_handle = handle;

    PRINTF("Connected to device:");
    for (uint32_t i = 5; i > 0; i--)
    {
        PRINTF("%02X-", addr[i]);
    }
    PRINTF("%02X\n", addr[0]);
}
