/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <onlplib/mmap.h>
#include "platform_lib.h"
#include "x86_64_delta_dsc100_int.h"
#include "x86_64_delta_i2c.h"
#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)
		

#define CPLD_NAME  "CPU_SWPLD"

#define CPLD_LED_SYS_REG                    (0X0a)

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        {ONLP_LED_ID_CREATE(LED_SYS), "sys", 0 },
        ONLP_LED_STATUS_PRESENT,
		ONLP_LED_CAPS_ON_OFF |
        ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_GREEN | 
		ONLP_LED_CAPS_RED | ONLP_LED_CAPS_RED_BLINKING |
		ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_ORANGE_BLINKING |
		ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING ,
    },
	
};

static int conver_led_light_mode_to_onl(uint32_t id, int led_ligth_mode)
{
    switch (id) {
    case LED_SYS:
		switch (led_ligth_mode) {
        case SYS_LED_MODE_OFF:     			    return ONLP_LED_MODE_OFF;
        case SYS_LED_MODE_GREEN:                return ONLP_LED_MODE_GREEN;
        case SYS_LED_MODE_GREEN_BLINKING:     	return ONLP_LED_MODE_GREEN_BLINKING;
        case SYS_LED_MODE_RED:    			    return ONLP_LED_MODE_RED;
		case SYS_LED_MODE_RED_BLINKING:			return ONLP_LED_MODE_RED_BLINKING;
		case SYS_LED_MODE_ORANGE:				return ONLP_LED_MODE_ORANGE;
		case SYS_LED_MODE_ORANGE_BLINKING:		return ONLP_LED_MODE_ORANGE_BLINKING;
		case SYS_LED_MODE_BLUE:					return ONLP_LED_MODE_BLUE;
		case SYS_LED_MODE_BLUE_BLINKING: 		return ONLP_LED_MODE_BLUE_BLINKING;
        }
    }

	return ONLP_LED_MODE_OFF;
}

static int conver_onlp_led_light_mode_to_driver(uint32_t id, int led_ligth_mode)
{
    switch (id) {
     case LED_SYS:
		switch (led_ligth_mode) {
        case ONLP_LED_MODE_OFF:     		    return SYS_LED_MODE_OFF;
        case ONLP_LED_MODE_GREEN:       		return SYS_LED_MODE_GREEN;
        case ONLP_LED_MODE_GREEN_BLINKING:      return SYS_LED_MODE_GREEN_BLINKING;
        case ONLP_LED_MODE_RED:  				return SYS_LED_MODE_RED;
		case ONLP_LED_MODE_RED_BLINKING:		return SYS_LED_MODE_RED_BLINKING;
		case ONLP_LED_MODE_ORANGE:				return SYS_LED_MODE_ORANGE;
		case ONLP_LED_MODE_ORANGE_BLINKING:		return SYS_LED_MODE_ORANGE_BLINKING;
		case ONLP_LED_MODE_BLUE: 				return SYS_LED_MODE_BLUE;
		case ONLP_LED_MODE_BLUE_BLINKING:		return SYS_LED_MODE_BLUE_BLINKING;
        }

    }

	return SYS_LED_MODE_UNKNOWN;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

static int
onlp_ledi_oid_to_internal_id(onlp_oid_t id)
{

	int lid = ONLP_OID_ID_GET(id);

	if ((product_id == PID_DSC100_LC) || (product_id == PID_DSC100_FC)){
		switch (lid) {
			case 1:	return LED_SYS;
		}
	}
	return lid;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{	
    int  r_data,m_data;
	
	VALIDATE(id);

    int lid = onlp_ledi_oid_to_internal_id(id);
	
	if((lid > LED_SYS)&& (lid<=LED_RESERVED))
		return ONLP_STATUS_E_INTERNAL;
    /* Set the onlp_oid_hdr_t and capabilities */		
    *info = linfo[lid];
	
    DEBUG_PRINT("id %u lid %d\n", id, lid);

    switch (lid)
    {
        case LED_SYS:
            r_data = i2c_devname_read_byte(CPLD_NAME, CPLD_LED_SYS_REG);
            if (r_data < 0)
                return ONLP_STATUS_E_INTERNAL;
            m_data = r_data;
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
    }
	
    info->mode = conver_led_light_mode_to_onl(lid, m_data);

        /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) 
        info->status |= ONLP_LED_STATUS_ON;
		
    return ONLP_STATUS_OK;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int  r_data,driver_mode, rc;
	int reg;
    
	VALIDATE(id);

    int lid = onlp_ledi_oid_to_internal_id(id);
	
	if((lid > LED_SYS)&& (lid<=LED_RESERVED))
		return ONLP_STATUS_E_INTERNAL;		

    driver_mode = conver_onlp_led_light_mode_to_driver(lid, mode);
	
	if(driver_mode==SYS_LED_MODE_UNKNOWN)
		return ONLP_STATUS_E_UNSUPPORTED;
	
    switch (lid)
    {
        case LED_SYS:
            reg = CPLD_LED_SYS_REG;
			r_data = driver_mode | 0x08;
			rc=i2c_devname_write_byte(CPLD_NAME, reg, r_data);
			if(rc<0){
				return ONLP_STATUS_E_INTERNAL;
			}
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
    }
    
 
    return ONLP_STATUS_OK;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
 */
int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{	
		if (!on_or_off) {
			return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
		}

    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

