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
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"
#include <unistd.h>

#include <x86_64_delta_dsc100/x86_64_delta_dsc100_config.h>
#include "x86_64_delta_dsc100_log.h"
#include "x86_64_delta_i2c.h"

#define QSFP_MIN_PORT 1
#define QSFP_MAX_PORT 16

#define QSFP_1_8_PRESENT_REG		(0X14)
#define QSFP_9_16_PRESENT_REG		(0X15)
#define QSFP_1_8_LPMOD_REG			(0X12)
#define QSFP_9_16_LPMOD_REG         (0X13)
#define QSFP_1_8_RESET_REG          (0X16)
#define QSFP_9_16_RESET_REG         (0X17)
#define INVALID_REG                 (0xFF)
#define INVALID_REG_BIT             (0xFF)

static int QSFP_INIT=0;

struct portCtrl{
    int portId;
    char  cpldName[32];
    int presentReg;
    int presentRegBit;
	int lpmodReg;
	int lpmodRegBit;
	int resetReg;
	int resetRegBit;
};

#define CPLD_NAME "CPU_SWPLD"

static struct portCtrl gPortCtrl[] = 
{
 {1, CPLD_NAME, QSFP_1_8_PRESENT_REG, 7,QSFP_1_8_LPMOD_REG,7,QSFP_1_8_RESET_REG,7},
 {2, CPLD_NAME, QSFP_1_8_PRESENT_REG, 6,QSFP_1_8_LPMOD_REG,6,QSFP_1_8_RESET_REG,6},
 {3, CPLD_NAME, QSFP_1_8_PRESENT_REG, 5,QSFP_1_8_LPMOD_REG,5,QSFP_1_8_RESET_REG,5},
 {4, CPLD_NAME, QSFP_1_8_PRESENT_REG, 4,QSFP_1_8_LPMOD_REG,4,QSFP_1_8_RESET_REG,4},
 {5, CPLD_NAME, QSFP_1_8_PRESENT_REG, 3,QSFP_1_8_LPMOD_REG,3,QSFP_1_8_RESET_REG,3},
 {6, CPLD_NAME, QSFP_1_8_PRESENT_REG, 2,QSFP_1_8_LPMOD_REG,2,QSFP_1_8_RESET_REG,2},
 {7, CPLD_NAME, QSFP_1_8_PRESENT_REG, 1,QSFP_1_8_LPMOD_REG,1,QSFP_1_8_RESET_REG,1},
 {8, CPLD_NAME, QSFP_1_8_PRESENT_REG, 0,QSFP_1_8_LPMOD_REG,0,QSFP_1_8_RESET_REG,0},

 {9,  CPLD_NAME, QSFP_9_16_PRESENT_REG, 7,QSFP_9_16_LPMOD_REG,7,QSFP_9_16_RESET_REG,7},
 {10, CPLD_NAME, QSFP_9_16_PRESENT_REG, 6,QSFP_9_16_LPMOD_REG,6,QSFP_9_16_RESET_REG,6},
 {11, CPLD_NAME, QSFP_9_16_PRESENT_REG, 5,QSFP_9_16_LPMOD_REG,5,QSFP_9_16_RESET_REG,5},
 {12, CPLD_NAME, QSFP_9_16_PRESENT_REG, 4,QSFP_9_16_LPMOD_REG,4,QSFP_9_16_RESET_REG,4},
 {13, CPLD_NAME, QSFP_9_16_PRESENT_REG, 3,QSFP_9_16_LPMOD_REG,3,QSFP_9_16_RESET_REG,3},
 {14, CPLD_NAME, QSFP_9_16_PRESENT_REG, 2,QSFP_9_16_LPMOD_REG,2,QSFP_9_16_RESET_REG,2},
 {15, CPLD_NAME, QSFP_9_16_PRESENT_REG, 1,QSFP_9_16_LPMOD_REG,1,QSFP_9_16_RESET_REG,1},
 {16, CPLD_NAME, QSFP_9_16_PRESENT_REG, 0,QSFP_9_16_LPMOD_REG,0,QSFP_9_16_RESET_REG,0},

 {0xFFFF, "", INVALID_REG, 0},
};

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{

	if(product_id == PID_DSC100_LC){
		
   		/* init the sfp module 
		disable the QSFP power*/
		i2c_devname_write_byte(CPLD_NAME,0x03,0x7f);

		/*enable QSFP in reset status*/
		i2c_devname_write_byte(CPLD_NAME,0x16,0x00);
		i2c_devname_write_byte(CPLD_NAME,0x17,0x00);

		/*enable QSFP I2C*/
		i2c_devname_write_byte(CPLD_NAME,0x10,0x00);
		i2c_devname_write_byte(CPLD_NAME,0x11,0x00);
	
		/*disable QSFP LP*/
		i2c_devname_write_byte(CPLD_NAME,0x12,0x00);
		i2c_devname_write_byte(CPLD_NAME,0x13,0x00);

		sleep(1);

		/*enable the QSFP power*/
		i2c_devname_write_byte(CPLD_NAME,0x03,0xff);

		sleep(1);

		/*disable QSFP out reset status*/
		i2c_devname_write_byte(CPLD_NAME,0x16,0xff);
		i2c_devname_write_byte(CPLD_NAME,0x17,0xff);
		QSFP_INIT =1;
	}
	
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{

	if(product_id == PID_DSC100_LC){
		
		int p;
		int start_port, end_port;

	    if(platform_id == PLATFORM_ID_DELTA_DSC100_R0)
    	{
        	start_port = QSFP_MIN_PORT;
        	end_port   = QSFP_MAX_PORT;
    	}
    	else /*reserved*/
    	{
			AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
			return ONLP_STATUS_E_UNSUPPORTED;
    	}

    	for(p = start_port; p <=end_port; p++) {
        	AIM_BITMAP_SET(bmap, p);
    	}
		return ONLP_STATUS_OK;
	}
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
	int present,r_data;

	if (QSFP_INIT == 0)
		onlp_sfpi_init();		
	
	if((port >= QSFP_MIN_PORT) && (port <= QSFP_MAX_PORT)){
		r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].presentReg);
   	}
	else{
		AIM_LOG_ERROR("The port %d is invalid \r\n", port);
		return ONLP_STATUS_E_UNSUPPORTED;
	}
		
	if(r_data<0){
			AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
			return ONLP_STATUS_E_INTERNAL;
	}
   	r_data = (~r_data) & 0xFF;

	present = (r_data >> gPortCtrl[port - 1].presentRegBit) & 0x1;
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{

	if(product_id == PID_DSC100_LC){
    	
		int status;
		int port, i = 0;
		uint64_t presence_all=0;

		AIM_BITMAP_CLR_ALL(dst);
	
	 	if(platform_id == PLATFORM_ID_DELTA_DSC100_R0)
	 	{		
			port = 16;
		
		}
		else{
        	AIM_LOG_ERROR("The platform id %d is invalid \r\n", platform_id);
			return ONLP_STATUS_E_UNSUPPORTED;
    	}
 
    	/*read 8 ports present status once*/
    	for (i = port; i >= QSFP_MIN_PORT;)
    	{
        	/*
        	AIM_LOG_ERROR("port %d, cpldname %s, reg %d\r\n", i, gPortCtrl[i - 1].cpldName, \
               gPortCtrl[i - 1].presentReg);
        	*/
        	status = i2c_devname_read_byte(gPortCtrl[i - 1].cpldName, gPortCtrl[i - 1].presentReg);

        	if(status<0){
            	AIM_LOG_ERROR("Unable to read presence from the port %d to %d value(status %d) \r\n", i, i + 8, status);
            	return ONLP_STATUS_E_INTERNAL;
        	}
        	status = ~(status) & 0xFF;
        	presence_all |= ((uint64_t)(status)) << (((16-i)/ 8) * 8);
        	i -= 8;
    	}

    	/* Populate bitmap */
    	for(i = port; presence_all; i--) {
        	AIM_BITMAP_MOD(dst, i, (presence_all & 1));
        	presence_all >>= 1;
    	}
		return ONLP_STATUS_OK;
	}
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
   return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    /*
     * Read the SFP eeprom into data[]
     *
     * Return MISSING if SFP is missing.
     * Return OK if eeprom is read
     */
	if(product_id == PID_DSC100_LC){
		int i;
		char sfp_name[32];
		//int i,re_cnt;uint8_t r_data;
    	memset(data, 0, 256);
    	memset(sfp_name, 0x0, sizeof(sfp_name));

		if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT)
    	{
        	AIM_LOG_ERROR("port %d is not invalid\r\n", port);
        	return ONLP_STATUS_E_INVALID;
    	}
    	if (onlp_sfpi_is_present(port) <= 0)
    	{
        	AIM_LOG_WARN("port %d is note present or error\r\n", port);
        	return ONLP_STATUS_E_MISSING;
    	}

       	sprintf(sfp_name, "QSFP%d", port);

    	for(i=0;i<8;i++){
        	if (i2c_devname_read_block(sfp_name, (32*i), (uint8_t*)(data+32*i), 32) < 0)
        	{
            	AIM_LOG_ERROR("Unable to read the port %d eeprom\r\n", port);
            	return ONLP_STATUS_E_INTERNAL;
        	}
    	}	
	    return ONLP_STATUS_OK;
	}
	return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    
    return onlp_sfpi_eeprom_read( port, data);
}

int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
	if(product_id == PID_DSC100_LC){
		char sfp_name[32];
		int r_data;
		if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT){
			AIM_LOG_ERROR("port %d is not invalid\r\n", port);
			return ONLP_STATUS_E_INVALID;
		}
		if (onlp_sfpi_is_present(port) <= 0){
			AIM_LOG_WARN("port %d is note present or error\r\n", port);
			return ONLP_STATUS_E_MISSING;
		}
		sprintf(sfp_name, "QSFP%d", port);
		r_data = i2c_devname_read_byte(sfp_name,addr);
		return r_data;
	}
	return ONLP_STATUS_E_UNSUPPORTED;

}

int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
	if(product_id == PID_DSC100_LC){
		char sfp_name[32];
		int ret;
		if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT){
			AIM_LOG_ERROR("port %d is not invalid\r\n", port);
			return ONLP_STATUS_E_INVALID;
		}
		if (onlp_sfpi_is_present(port) <= 0){
			AIM_LOG_WARN("port %d is note present or error\r\n", port);
			return ONLP_STATUS_E_MISSING;
		}
		sprintf(sfp_name, "QSFP%d", port);
		ret = i2c_devname_write_byte(sfp_name,addr,value);
		return ret;
	}
	return ONLP_STATUS_E_UNSUPPORTED;
}


int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
	/*Returns whether or not the given control is suppport on the given port
	 if supported *rv=1
	 if not supported *rv =0
	*/
	  if(product_id == PID_DSC100_LC){
		int ret;
		*rv = 0;
		if (port < QSFP_MIN_PORT || port > QSFP_MAX_PORT){
			AIM_LOG_ERROR("port %d is not invalid\r\n", port);
			return ONLP_STATUS_E_INVALID;
		}
		if (control > ONLP_SFP_CONTROL_LAST)
			return ONLP_STATUS_E_INVALID;

		ret = onlp_sfpi_is_present(port);

		if(ret <=0)
			return ONLP_STATUS_OK;

		switch (control) {
			case ONLP_SFP_CONTROL_RESET:
			case ONLP_SFP_CONTROL_LP_MODE:
				*rv = 1;
				break;
			default:
				break;
		}
		return ONLP_STATUS_OK;
	 }
	 return ONLP_STATUS_E_UNSUPPORTED;
}

static int sfp_control_set(int port,onlp_sfp_control_t control, int value)
{
	int ret;
	int r_data,w_data;

	if((port < QSFP_MIN_PORT) || (port > QSFP_MAX_PORT)){
		AIM_LOG_ERROR("port %d is not invalid\r\n", port);
		return  -1 ;
	}
	switch (control) {
		case ONLP_SFP_CONTROL_LP_MODE:
			r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpmodReg);
			if (r_data <0)
				ret =-1;
			r_data &= ~(1<<gPortCtrl[port - 1].lpmodRegBit);
			w_data = r_data | (value<<gPortCtrl[port - 1].lpmodRegBit);
			ret = i2c_devname_write_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpmodReg, w_data);
			break;
		case ONLP_SFP_CONTROL_RESET:
			r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg);
			if (r_data <0)
				ret =-1;
			r_data &= ~(1<<gPortCtrl[port - 1].resetRegBit);
			w_data = r_data | (value<<gPortCtrl[port - 1].resetRegBit);
			ret = i2c_devname_write_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg, w_data);
			break;
		default:
			ret =-1;
			break;
	}
	return ret;	
}

static int sfp_control_get(int port,onlp_sfp_control_t control)
{
	int ret;
	int r_data;
	if((port < QSFP_MIN_PORT) || (port > QSFP_MAX_PORT)){
		AIM_LOG_ERROR("port %d is not invalid\r\n", port);
		return  -1 ;
	}
	switch (control) {
		case ONLP_SFP_CONTROL_LP_MODE:
			r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].lpmodReg);
			if (r_data <0)
				ret =-1;
			r_data =(r_data>>gPortCtrl[port - 1].lpmodRegBit);
			ret = r_data;
			break;
		case ONLP_SFP_CONTROL_RESET:
			r_data=i2c_devname_read_byte(gPortCtrl[port - 1].cpldName, gPortCtrl[port - 1].resetReg);
			if (r_data <0)
				ret =-1;
			r_data =(r_data>>gPortCtrl[port - 1].resetRegBit);
			ret = r_data;
			break;
		default:
			ret =-1;
			break;
	}
	return ret;
}

int
onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
	
	if(product_id == PID_DSC100_LC){
		int contrl_support=0;	
		int ret;
		onlp_sfpi_control_supported(port, control, &contrl_support);
		if(!contrl_support)
			return ONLP_STATUS_E_UNSUPPORTED;
		ret = sfp_control_set(port, control,value);
		if(ret<0)
			return ONLP_STATUS_E_INVALID;
		return ONLP_STATUS_OK;

	}
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
	if(product_id == PID_DSC100_LC){
		int contrl_support=0;
		int ret;
		onlp_sfpi_control_supported(port, control, &contrl_support);
		if(!contrl_support)
			return ONLP_STATUS_E_UNSUPPORTED;

		ret = sfp_control_get(port, control);
		
		if(ret<0)
			return ONLP_STATUS_E_INVALID;

		*value = ret;

		return ONLP_STATUS_OK;
	}
    return ONLP_STATUS_E_UNSUPPORTED;
}


int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
