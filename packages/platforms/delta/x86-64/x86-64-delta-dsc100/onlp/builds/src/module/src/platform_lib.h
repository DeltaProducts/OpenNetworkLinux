/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_delta_dsc100_log.h"

#define CHASSIS_FAN_COUNT		0
#define CHASSIS_THERMAL_COUNT	5
#define CHASSIS_PSU_COUNT	    0
#define CHASSIS_LED_COUNT		1

#define CHASSIS_LC_MIN_SLOT		0X01
#define CHASSIS_LC_MAX_SLOT     0X08
#define CHASSIS_FC_MIN_SLOT     0X09 
#define CHASSIS_FC_MAX_SLOT     0X0C

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)  \
    {\
        printf("[%s:%d] ", __FUNCTION__, __LINE__);\
        printf(format, __VA_ARGS__); \
    }
#else
    #define DEBUG_PRINT(format, ...)
#endif

typedef enum dsc100_product_id 
{	
	PID_RESERVED = 0,
	PID_DSC100_LC = 1,
	PID_DSC100_FC = 2,
	PID_UNKNOWN
} product_id_t;

/* LED related data */
enum sys_led_light_mode {
	SYS_LED_MODE_OFF = 0x0,
	SYS_LED_MODE_GREEN = 0x01,
	SYS_LED_MODE_GREEN_BLINKING = 0x11,
	SYS_LED_MODE_RED = 0x02,
	SYS_LED_MODE_RED_BLINKING = 0x12,
	SYS_LED_MODE_ORANGE = 0x03,
	SYS_LED_MODE_ORANGE_BLINKING = 0x13,
	SYS_LED_MODE_BLUE= 0x04,
	SYS_LED_MODE_BLUE_BLINKING = 0x14,
	SYS_LED_MODE_UNKNOWN
};

typedef enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYS,
} onlp_led_id_t;


typedef enum platform_id_e {
    PLATFORM_ID_UNKNOWN = 0,
    PLATFORM_ID_DELTA_DSC100_R0,
} platform_id_t;

extern platform_id_t platform_id;
extern product_id_t product_id;

product_id_t get_product_id(void);
#endif  /* __PLATFORM_LIB_H__ */
