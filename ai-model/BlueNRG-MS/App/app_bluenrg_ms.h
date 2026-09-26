/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_BLUENRG_MS_H
#define APP_BLUENRG_MS_H

#include "hci.h"
#include "hci_le.h"
#include "hci_tl.h"
#include "link_layer.h"
#include "compiler.h"
#include "bluenrg_utils.h"
#include "bluenrg_gap.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_aci_const.h"
#include "sm.h"

#include <stdlib.h>
#include "hci_const.h"

#include "heatstroke_service.h"

#define PROJECT_NAME 'H', 'e', 'a', 't'
#define BDADDR_SIZE 6
#define ADV_INTERVAL_MIN_MS 1000
#define ADV_INTERVAL_MAX_MS 1200

void IDB05A2_BlueNRG_MS_Init(void);
void MX_BlueNRG_MS_Process(float risk_score, float env_temp, float skin_temp, float RH,
						   float exp_to_sun, float heart_rate);
void user_notify(void *pData);
void transmit_heatstroke_data(float risk_score, float env_temp, float skin_temp, float RH,
							  float exp_to_sun, float heart_rate);
#endif /* APP_BLUENRG_MS_H */
