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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include "x86_64_delta_i2c.h"

product_id_t get_product_id(void)
{
	int r_data;
	r_data = i2c_devname_read_byte("CPU_SWPLD",0X00);

	if(r_data <0)
		return PID_UNKNOWN;

	r_data = r_data>>4;

	if((r_data <= CHASSIS_LC_MAX_SLOT) && (r_data >= CHASSIS_LC_MIN_SLOT))
		return PID_DSC100_LC;
	else if ((r_data <= CHASSIS_FC_MAX_SLOT) && (r_data >= CHASSIS_FC_MIN_SLOT))
 		return PID_DSC100_FC;
	else
		return	PID_UNKNOWN;
}


