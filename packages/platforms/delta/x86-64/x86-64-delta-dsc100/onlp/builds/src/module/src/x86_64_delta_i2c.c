/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
 ************************************************************/
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "x86_64_delta_dsc100_log.h"
#include "x86_64_delta_i2c.h"
#include <onlplib/i2c.h>
struct i2c_device_info i2c_device_list[]={
	{"CPUPLD",6,0X30},
	{"CPU_SWPLD",7,0X31},
	{"BMC_SWPLD",11,0X31},
 	{"TMP_CLOSE_TO_CPU",12,0X4F},
 	{"TMP_CLOSE_TO_INLET",12,0X4D},
	{"TMP_CLOSE_TO_MAC",12,0X4C},
	{"TMP_CLOSE_TO_BMC",12,0X4A},
	{"EEPROM-MB",11,0X50},
 	{"EEPROM-CPU",10,0X50},
 	{"EEPROM-ONIE",0,0X56},
 	{"IR3595",11,0X70},
 	{"E-FUSE",11,0X20},
 	{"MUX-PCA9544",2,0X70},
    {"QSFP1",13,0X50},
    {"QSFP2",14,0X50},
    {"QSFP3",15,0X50},
    {"QSFP4",16,0X50},
    {"QSFP5",17,0X50},
    {"QSFP6",18,0X50},
    {"QSFP7",19,0X50},
    {"QSFP8",20,0X50},
    {"QSFP9",21,0X50},
    {"QSFP10",22,0X50},
    {"QSFP11",23,0X50},
    {"QSFP12",24,0X50},
    {"QSFP13",25,0X50},
    {"QSFP14",26,0X50},
    {"QSFP15",27,0X50},
    {"QSFP16",28,0X50},

    {NULL,  -1,-1},
};

uint32_t i2c_flag=ONLP_I2C_F_FORCE;

i2c_device_info_t *i2c_dev_find_by_name (char *name)
{
	i2c_device_info_t *i2c_dev = i2c_device_list;

	if (name == NULL) return NULL;

	while (i2c_dev->name) {
		if (strcmp (name, i2c_dev->name) == 0) break;
		++ i2c_dev;
	}
	if (i2c_dev->name == NULL) return NULL;

	return i2c_dev;
}

int i2c_devname_read_byte  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

    
	if(i2c_dev==NULL) return -1;

	
	ret=onlp_i2c_readb(i2c_dev->i2cbus, i2c_dev->addr, reg, i2c_flag);	


	return ret;
}

int i2c_devname_write_byte (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	 if(i2c_dev==NULL) return -1;

	
	ret=onlp_i2c_writeb (i2c_dev->i2cbus, i2c_dev->addr, reg, value, i2c_flag);


	return ret;
}

int i2c_devname_read_word  (char *name, int reg)
{	
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);

	if(i2c_dev==NULL) return -1;
	
	ret=onlp_i2c_readw(i2c_dev->i2cbus, i2c_dev->addr, reg, i2c_flag);	


	return ret;
}

int i2c_devname_write_word (char *name, int reg, int value)
{
	int ret=-1;
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
	
	if(i2c_dev==NULL) return -1;

	
	ret=onlp_i2c_writew (i2c_dev->i2cbus, i2c_dev->addr, reg, value, i2c_flag);


	return ret;
}

int i2c_devname_read_block (char *name, int reg, uint8_t*buff, int buff_size)
{	
	int ret = -1;
	
	i2c_device_info_t *i2c_dev = i2c_dev_find_by_name (name);
		
	if(i2c_dev==NULL) return -1;
	
	
	ret =onlp_i2c_block_read (i2c_dev->i2cbus, i2c_dev->addr, reg, buff_size, buff, i2c_flag);


	return ret;

}

