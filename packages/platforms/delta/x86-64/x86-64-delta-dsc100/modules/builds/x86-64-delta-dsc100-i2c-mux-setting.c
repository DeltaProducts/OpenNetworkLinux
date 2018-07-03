
/*
 *
 * Copyright (C) 2017 Delta Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#if 0
#include "x86-64-delta-dsc100-i2c-mux-cpld.h"
#else
struct i2c_mux_cpld_platform_data
{
	u8 cpld_bus;
	u8 cpld_addr;
	u8 cpld_reg;

	u8 parent_bus;

	u8 base_nr;

	const u8 *values;
	int n_values;
	bool idle_in_use;
	u8  idle;

	void *ctrl_adap;
};
#endif
static const u8 qsfp_mux_values_1_8[] = {
	 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};
static struct i2c_mux_cpld_platform_data qsfp_mux_1_8 = {
	.cpld_bus = 7,
	.cpld_addr= 0x31,
	.cpld_reg = 0x1a,

	.parent_bus = 0,

	.values = qsfp_mux_values_1_8,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,
};
static const u8 qsfp_mux_values_9_16[] = {
	 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16
};
static struct i2c_mux_cpld_platform_data qsfp_mux_9_16 = {
	.cpld_bus = 7,
	.cpld_addr= 0x31,
	.cpld_reg = 0x1a,

	.parent_bus = 0,

	.values = qsfp_mux_values_9_16,
	.n_values = 8,
	.idle_in_use = 1,
	.idle = 0xff,

};

static int add_dsc100_platform_cpld_mux_devices(void)
{
	struct platform_device *pdev;

	pdev = platform_device_register_simple("i2c-mux-cpld", 1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_1_8, sizeof(qsfp_mux_1_8));

	pdev = platform_device_register_simple("i2c-mux-cpld", 2, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	platform_device_add_data (pdev, &qsfp_mux_9_16, sizeof(qsfp_mux_9_16));

	return 0;
}

static int __init dsc100_platform_init(void)
{
	add_dsc100_platform_cpld_mux_devices ();

        return 0;
}

static void __exit dsc100_platform_exit(void)
{
}

MODULE_AUTHOR("Dave Hu <dave.hu@deltaww.com>");
MODULE_DESCRIPTION("Delta DSC100");
MODULE_LICENSE("GPL");

module_init(dsc100_platform_init);
module_exit(dsc100_platform_exit);


