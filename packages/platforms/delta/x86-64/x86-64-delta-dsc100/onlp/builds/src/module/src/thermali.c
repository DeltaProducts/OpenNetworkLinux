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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <fcntl.h>
#include "platform_lib.h"
#include "x86_64_delta_dsc100_log.h"
#include <stdio.h>

#define LOCAL_DEBUG 0
#define prefix_path "/sys/bus/i2c/devices/"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
  	THERMAL_CLOSE_TO_CPU,
    THERMAL_CLOSE_TO_INLET,
	THERMAL_CLOSE_TO_MAC,
	THERMAL_CLOSE_TO_BMC,
	THERMAL_TO_CPU,
};

typedef enum thermal_threshold_e {
    THERMAL_THRESHOLD_WARNING_1 = 78000,
	THERMAL_THRESHOLD_WARNING_2 = 75000,
	THERMAL_THRESHOLD_WARNING_3 = 70000,
	THERMAL_THRESHOLD_WARNING_4 = 94000,
	THERMAL_THRESHOLD_ERROR_1 = 83000,
	THERMAL_THRESHOLD_ERROR_2 = 85000,
	THERMAL_THRESHOLD_ERROR_3 = 98000,
	THERMAL_THRESHOLD_SHUTDOWN_1 = 88000,
	THERMAL_THRESHOLD_SHUTDOWN_2 = 99000,
} thermal_threshold_t;

#define CPU_THERMAL_THRESHOLD_INIT          \
    {THERMAL_THRESHOLD_WARNING_1 ,          \
	 THERMAL_THRESHOLD_ERROR_2,             \
	 THERMAL_THRESHOLD_SHUTDOWN_1 }

#define MAC_THERMAL_THRESHOLD_INIT          \
	{THERMAL_THRESHOLD_WARNING_2 ,          \
	 THERMAL_THRESHOLD_ERROR_1,             \
	 THERMAL_THRESHOLD_SHUTDOWN_1 }


#define BMC_THERMAL_THRESHOLD_INIT          \
	{THERMAL_THRESHOLD_WARNING_2 ,          \
	 THERMAL_THRESHOLD_ERROR_1,             \
	 THERMAL_THRESHOLD_SHUTDOWN_1 }


#define INLET_THERMAL_THRESHOLD_INIT          \
	{THERMAL_THRESHOLD_WARNING_3 ,          \
	 THERMAL_THRESHOLD_ERROR_1,             \
	 THERMAL_THRESHOLD_SHUTDOWN_1 }

#define CORE_CPU_THERMAL_THRESHOLD_INIT          \
	{THERMAL_THRESHOLD_WARNING_4 ,          \
	 THERMAL_THRESHOLD_ERROR_3,             \
	 THERMAL_THRESHOLD_SHUTDOWN_2 }

static char* last_path[] =  /* must map with onlp_thermal_id */
{
    "reserved",
	"12-004f/hwmon/hwmon1/temp1_input",
	"12-004d/hwmon/hwmon2/temp1_input",
	"12-004c/hwmon/hwmon3/temp1_input",
	"12-004a/hwmon/hwmon4/temp1_input",
};
/*
#define THERMAL_NAME1 "TMP_CLOSE_TO_CPU"
#define THERMAL_NAME2 "TMP_CLOSE_TO_INLET"
#define THERMAL_NAME3 "TMP_CLOSE_TO_MAC"
#define THERMAL_NAME4 "TMP_CLOSE_TO_BMC"
*/
/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CLOSE_TO_CPU), "Thermal Sensor 1- close to cpu", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CPU_THERMAL_THRESHOLD_INIT
    },
    { { ONLP_THERMAL_ID_CREATE(THERMAL_CLOSE_TO_INLET), "Thermal Sensor 2- close to inlet", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, INLET_THERMAL_THRESHOLD_INIT
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CLOSE_TO_MAC), "Thermal Sensor 3- close to mac", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, MAC_THERMAL_THRESHOLD_INIT
    },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CLOSE_TO_BMC), "Thermal Sensor 4- close to bmc", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, BMC_THERMAL_THRESHOLD_INIT
    },

	{ { ONLP_THERMAL_ID_CREATE(THERMAL_TO_CPU), "Thermal Sensor 5- cpu core", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, CORE_CPU_THERMAL_THRESHOLD_INIT
    },

};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{ 
    return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
static int
_onlp_thermali_sensor_info_get(int local_id, onlp_thermal_info_t* info)
{
    int len, nbytes = 10, temp_base=1;
    uint8_t r_data[10]={0};
    char  fullpath[50] = {0};

    DEBUG_PRINT("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];
    /* get fullpath */
    sprintf(fullpath, "%s%s", prefix_path, last_path[local_id]);

    //OPEN_READ_FILE(fd, fullpath, r_data, nbytes, len);
    onlp_file_read(r_data,nbytes,&len, fullpath);
    
    info->mcelsius =ONLPLIB_ATOI((char*)r_data) / temp_base;
    
    DEBUG_PRINT("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);

    return ONLP_STATUS_OK;
}

static int
_onlp_thermali_cpu_info_get(int local_id,onlp_thermal_info_t*info)
{
	char r_data[10]={0};
	int temp_base=1000;
	char cpu_core_name[10] = {0};
	char cmd[100] ={0};
	FILE* fp = NULL;
	DEBUG_PRINT("\n[Debug][%s][%d][local_id: %d]", __FUNCTION__, __LINE__, local_id);
	/* Set the onlp_oid_hdr_t and capabilities */
	*info = linfo[local_id];
	/* get cpu core name  */
	sprintf(cpu_core_name, "Core %d", local_id-THERMAL_TO_CPU);
	sprintf(cmd, "sensors | grep '%s' %s", cpu_core_name,"| awk  '{print $3}' | awk -F '°' '{print $1}' | awk -F '+' '{print $2}'");
	fp = popen(cmd, "r");
	if(fp == NULL)
		return ONLP_STATUS_E_INTERNAL;
	if(fgets(r_data, sizeof(r_data), fp) == NULL)
		return ONLP_STATUS_E_INTERNAL;
    pclose(fp);
	info->mcelsius =ONLPLIB_ATOI(r_data) * temp_base;
	DEBUG_PRINT("\n[Debug][%s][%d][save data: %d]\n", __FUNCTION__, __LINE__, info->mcelsius);
	return ONLP_STATUS_OK;
}

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
	int local_id;
	VALIDATE(id);
	int ret;
	local_id = ONLP_OID_ID_GET(id);
	if((local_id> THERMAL_TO_CPU) || (local_id < THERMAL_CLOSE_TO_CPU)){
		DEBUG_PRINT("\n[Debug][%s][%d][local_id %d: is invalid]", __FUNCTION__, __LINE__, local_id);
		info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
		return ONLP_STATUS_E_INTERNAL;
	}
	if (local_id == THERMAL_TO_CPU)
		ret=_onlp_thermali_cpu_info_get(local_id,info);
	else
	   	ret=_onlp_thermali_sensor_info_get(local_id,info);
	if (ret <0)
		return ONLP_STATUS_E_INTERNAL;		
	return ONLP_STATUS_OK;
}

