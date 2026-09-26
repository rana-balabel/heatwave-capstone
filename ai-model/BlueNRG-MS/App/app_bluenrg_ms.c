#include "app_bluenrg_ms.h"

extern uint32_t connected;
extern uint8_t set_connectable;

uint32_t last_update_tick = 0;
uint8_t bdaddr[BDADDR_SIZE];

void IDB05A2_BlueNRG_MS_Init(void)
{
    /* USER CODE BEGIN SV */

    /* USER CODE END SV */

    /* USER CODE BEGIN BlueNRG_MS_Init_PreTreatment */

    /* USER CODE END BlueNRG_MS_Init_PreTreatment */

    /* Initialize the peripherals and the BLE Stack */
    const char *name = "Heat";
    uint16_t service_handle, dev_name_char_handle, appearance_char_handle;

    uint8_t bdaddr_len_out;
    int ret;

    hci_init(user_notify, NULL);

    /*
     * Reset BlueNRG again otherwise we won't
     * be able to change its MAC address.
     * aci_hal_write_config_data() must be the first
     * command after reset otherwise it will fail.
     */
    hci_reset();
    HAL_Delay(100);

    ret = aci_hal_read_config_data(CONFIG_DATA_RANDOM_ADDRESS, BDADDR_SIZE, &bdaddr_len_out, bdaddr);

    if (ret)
    {
        PRINTF("Read Static Random address failed.\n");
    }

    /* GATT */
    ret = aci_gatt_init();
    if (ret)
    {
        PRINTF("GATT_Init failed.\n");
    }

    /* GAP Init */
    // The GAP profile is used to initialize the stack and setup the connection with other devices.

    ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

    if (ret != BLE_STATUS_SUCCESS)
    {
        PRINTF("GAP_Init failed.\n");
    }

    /* Device name */
    aci_gatt_update_char_value(service_handle,
                               dev_name_char_handle,
                               0,
                               strlen(name),
                               (uint8_t *)name);

    PRINTF("BLE Stack Initialized\n");

    /* Add Heatstroke service + characteristics */
    ret = Add_Heatstroke_Service();
    if (ret != BLE_STATUS_SUCCESS)
    {
        PRINTF("Error adding Heatstroke service: 0x%02X\r\n", ret);
        while (1)
            ;
    }

    aci_hal_set_tx_power_level(1, 4);

    if (set_connectable)
    {
        Set_DeviceConnectable();
        set_connectable = FALSE;
    }
}
/**
 * @brief  Callback processing the ACI events.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void user_notify(void *pData)
{
    hci_uart_pckt *hci_pckt = pData;
    /* obtain event packet */
    hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;

    if (hci_pckt->type != HCI_EVENT_PKT)
        return;

    switch (event_pckt->evt)
    {
    case EVT_DISCONN_COMPLETE:
        GAP_DisconnectionComplete_CB();
        break;

    case EVT_LE_META_EVENT:
    {
        evt_le_meta_event *evt = (void *)event_pckt->data;
        if (evt->subevent == EVT_LE_CONN_COMPLETE)
        {
            evt_le_connection_complete *cc = (void *)evt->data;
            GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        }
        break;
    }

    case EVT_VENDOR:
    {
        evt_blue_aci *blue_evt = (void *)event_pckt->data;

        if (blue_evt->ecode == EVT_BLUE_GATT_READ_PERMIT_REQ)
        {
            evt_gatt_read_permit_req *pr = (void *)blue_evt->data;
            Read_Request_CB(pr->attr_handle);
        }
        break;
    }
    }
}

void Set_DeviceConnectable(void)
{
    const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME, PROJECT_NAME};

    hci_le_set_scan_resp_data(0, NULL);

    tBleStatus ret = aci_gap_set_discoverable(ADV_DATA_TYPE,
                                              (ADV_INTERVAL_MIN_MS * 1000) / 625,
                                              (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                              STATIC_RANDOM_ADDR,
                                              NO_WHITE_LIST_USE,
                                              sizeof(local_name),
                                              local_name,
                                              0,
                                              NULL,
                                              0,
                                              0);
    if (ret != BLE_STATUS_SUCCESS)
    {
        PRINTF("Set_DeviceConnectable failed.\n");
    }
}

void MX_BlueNRG_MS_Process(float risk_score, float env_temp, float skin_temp, float RH,
                           float exp_to_sun, float heart_rate)
{
    Heatstroke_SetValues(risk_score, env_temp, skin_temp, RH, exp_to_sun, heart_rate);

    transmit_heatstroke_data(risk_score, env_temp, skin_temp, RH,
                             exp_to_sun, heart_rate);

    hci_user_evt_proc();
}

void transmit_heatstroke_data(float risk_score, float env_temp, float skin_temp, float RH,
                              float exp_to_sun, float heart_rate)
{
    // every 5 seconds - reduced to prevent UART overload during debugging
    // todo: need to modify this logic with state machine
    const uint32_t update_period_ms = 5000;

    // if peer device gets disconnected
    if (set_connectable)
    {
        Set_DeviceConnectable();
        set_connectable = FALSE;
    }

    if (connected && (HAL_GetTick() - last_update_tick) > update_period_ms)
    {
        last_update_tick = HAL_GetTick();

        tBleStatus ret_risk = Heatstroke_Update(risk_score);
        tBleStatus ret_temp = EnvTemp_Update(env_temp);
        tBleStatus ret_Skintemp = SkinTemp_Update(skin_temp);
        tBleStatus ret_RH = Humidity_Update(RH);
        tBleStatus ret_expToSun = ExpToSun_Update(exp_to_sun);
        tBleStatus ret_HeartRate = HeartRate_Update(heart_rate);

        if (ret_risk == BLE_STATUS_SUCCESS)
            PRINTF("BLE risk updated: %.2f\r\n", risk_score);

        if (ret_temp == BLE_STATUS_SUCCESS)
            PRINTF("BLE env temp updated: %.2f\r\n", env_temp);

        if (ret_Skintemp == BLE_STATUS_SUCCESS)
            PRINTF("BLE skin temp updated: %.2f\r\n", skin_temp);

        if (ret_RH == BLE_STATUS_SUCCESS)
            PRINTF("BLE RH updated: %.2f\r\n", RH);

        if (ret_expToSun == BLE_STATUS_SUCCESS)
            PRINTF("BLE expToSun updated: %.2f\r\n", exp_to_sun);

        if (ret_HeartRate == BLE_STATUS_SUCCESS)
            PRINTF("BLE heartRate updated: %.2f\r\n", heart_rate);
    }
}
