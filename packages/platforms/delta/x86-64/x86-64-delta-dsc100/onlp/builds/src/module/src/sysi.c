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
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_delta_dsc100_int.h"
#include "x86_64_delta_dsc100_log.h"

#include "platform_lib.h"
#include "x86_64_delta_i2c.h"

platform_id_t platform_id = PLATFORM_ID_UNKNOWN;
product_id_t product_id = PID_UNKNOWN;

#define ONIE_PLATFORM_NAME "x86-64-delta-dsc100"

static int SYS_INT=0;

const char*
onlp_sysi_platform_get(void)
{ 
	product_id = get_product_id();
	
	if ((product_id == PID_DSC100_LC) || (product_id == PID_DSC100_FC))
		return ONIE_PLATFORM_NAME;
	else 
		return "unknow";
}

int
onlp_sysi_platform_set(const char* platform)
{ 
    if(strstr(platform,"x86-64-delta-dsc100-r0")) {
        platform_id = PLATFORM_ID_DELTA_DSC100_R0;
        return ONLP_STATUS_OK;
    }
	AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{ 
     char buff[128];
     int swpld_v,cpupld_v;
     int cpu_board_v,main_board_v;
     /*get the cpld version*/
     swpld_v = i2c_devname_read_byte("CPU_SWPLD", 0X3f);
     cpupld_v = i2c_devname_read_byte("CPUPLD",0x01);
     /*get the board hardware version*/
     cpu_board_v = i2c_devname_read_byte("CPUPLD",0x02);
     cpu_board_v = (cpu_board_v & 0xf0) >> 4;

     main_board_v = i2c_devname_read_byte("CPUPLD",0x04);
     main_board_v = main_board_v & 0x0f;

     snprintf (buff, sizeof(buff),"CpuPldVer=%d,SwPldVer=%d",cpupld_v,swpld_v);
     pi->cpld_versions = aim_fstrdup("%d", buff);

     snprintf (buff, sizeof(buff),"CpuBoardHwVer=%d,MainBoardHwVer=%d",cpu_board_v,main_board_v);
     pi->cpld_versions = aim_fstrdup("%d", buff);

    return 0;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
	int i,re_cnt;
    uint8_t* rdata = aim_zmalloc(256);
    if(!rdata){
		AIM_LOG_ERROR("Unable to malloc memory \r\n");
		return ONLP_STATUS_E_INTERNAL;
	}
	for(i=0;i<8;i++){
		re_cnt=3;
		while(re_cnt){
			if (i2c_devname_read_block("EEPROM-ONIE", i * 32, (rdata + i * 32), 32) < 0)
			{
                re_cnt--;
				continue;
			}
			break;
		}
		if(re_cnt==0){
			AIM_LOG_ERROR("Unable to read the %d reg \r\n",i);
			break;
		}
			
	}
   
    *data = rdata;

    return ONLP_STATUS_OK;

	
}

int
onlp_sysi_init(void)
{
    if(SYS_INT==0){
	    /*set the sys led to green*/
        i2c_devname_write_byte("CPU_SWPLD",0x0a,0x01|0x08);
	    i2c_devname_write_byte("BMC_SWPLD",0x0a,0x01|0x08);
        /*close down the eth2 and eth3*/
        system("ifconfig eth2 down > /dev/null 2>&1");
        system("ifconfig eth3 down > /dev/null 2>&1");
        SYS_INT=1;
    }
	return ONLP_STATUS_OK;
}


void
onlp_sysi_onie_data_free(uint8_t* data)
{ 
    aim_free(data);
}



int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{ 
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 1 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 1 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}
int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{   
    if(onie){
        onie->platform_name = aim_strdup(ONIE_PLATFORM_NAME);
    }
    return ONLP_STATUS_OK;
}



int
onlp_sysi_platform_manage_fans(void)
{ 
    return ONLP_STATUS_OK;
}


int
onlp_sysi_platform_manage_leds(void)
{ 
	return ONLP_STATUS_OK;
}

