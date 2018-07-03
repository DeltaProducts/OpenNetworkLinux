#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/completion.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/ipmi.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/compat.h>
#include <linux/list.h>

#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/atomic.h>
#include <linux/delay.h>

#define NETFN             0x30
#define CMD_FOR_I2C       0x52

#define I2C_IPMI_MAX_BUS  128
static int bmc_bus[I2C_IPMI_MAX_BUS];
static int bmc_bus_num;
module_param_array(bmc_bus, int, &bmc_bus_num, 0);
MODULE_PARM_DESC(bmc_bus, "Mapping BMC bus id");

struct i2c_ipmi_private
{
	ipmi_user_t         user;

    struct mutex        i2c_bus_lock;
    struct completion   i2c_bus_wait;

    atomic_t            i2c_bus_tofree;
};

struct i2c_ipmi_algo_data_t
{
	int	bmc_bus;		/* which bus, i2c bus id on BMC */

	struct i2c_ipmi_private ipmi;
};

static void i2c_ipmi_free_recv(struct ipmi_recv_msg *msg)
{
    struct i2c_ipmi_private *priv = msg->user_msg_data;

    if (atomic_dec_and_test(&priv->i2c_bus_tofree)) {
        complete(&priv->i2c_bus_wait);
    }
}

static struct i2c_ipmi_algo_data_t	i2c_ipmi_adap_data[I2C_IPMI_MAX_BUS];
static struct i2c_adapter 		i2c_ipmi_adap[I2C_IPMI_MAX_BUS];

static void i2c_ipmi_receive_handler (struct ipmi_recv_msg *msg,
				  void                 *handler_data)
{
	ipmi_free_recv_msg(msg);
}

static struct ipmi_user_hndl ipmi_hndlrs = {
	.ipmi_recv_hndl = i2c_ipmi_receive_handler,
};

static int i2c_ipmi_open(struct i2c_ipmi_private *priv)
{
    int     if_num = 0;
	int     rv = 0;

	rv = ipmi_create_user(if_num, &ipmi_hndlrs, priv, &priv->user);
	if (rv) {
		goto out;
	}

    atomic_set(&priv->i2c_bus_tofree, 0);
    init_completion(&priv->i2c_bus_wait);
    mutex_init(&priv->i2c_bus_lock);

    return 0;

out:
	return rv;
}

static int i2c_ipmi_close(struct i2c_ipmi_private *priv)
{
	int     rv = 0;

	rv = ipmi_destroy_user (priv->user);
	if (rv) {
		goto out;
	}
out:
	return rv;
}


static int _i2c_ipmi_send_recv (struct i2c_ipmi_private *priv,
		u8 bus, u8 i2c_addr, u8 *rd, u8 rc, u8 *wd, u8 wc)
{
	struct ipmi_recv_msg *i2c_ipmi_recv_msg;
	struct kernel_ipmi_msg  msg;
	int                     rv = -EIO;
	unsigned char           buff[128];

	struct ipmi_system_interface_addr addr;

	i2c_ipmi_recv_msg = (struct ipmi_recv_msg *)kmalloc(sizeof(*i2c_ipmi_recv_msg), GFP_KERNEL);
	if (!i2c_ipmi_recv_msg) {
		return -ENOMEM;
	}

	memset (i2c_ipmi_recv_msg, 0, sizeof(*i2c_ipmi_recv_msg));

	i2c_ipmi_recv_msg->done      = i2c_ipmi_free_recv;

	addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
	addr.channel = IPMI_BMC_CHANNEL;
	addr.lun = 0;

	msg.netfn = NETFN;;
	msg.cmd = CMD_FOR_I2C;
	msg.data_len = 3 + wc;;
	msg.data = buff;

	buff[0] = bus;
	buff[1] = i2c_addr; /* 8-bit format */
	buff[2] = rc;
	memcpy (&buff[3], wd, wc);

	mutex_lock(&priv->i2c_bus_lock);

	atomic_set(&priv->i2c_bus_tofree, 1);

	rv = ipmi_request_supply_msgs(priv->user,
				      (struct ipmi_addr *) &addr,
				      0,
				      &msg,
				      priv,
                      NULL,
				      i2c_ipmi_recv_msg,
				      1);
	if (rv) {
        mutex_unlock(&priv->i2c_bus_lock);
		printk(KERN_WARNING "i2c ipmi failure: %d\n",
		       rv);
		goto out;
	}

	/* Wait for the heartbeat to be sent. */
	wait_for_completion(&priv->i2c_bus_wait);

	if (i2c_ipmi_recv_msg->msg.data[0] == 0)  {
        memcpy (rd, &i2c_ipmi_recv_msg->msg.data[1], rc);
        rv = 0;
	} else if (i2c_ipmi_recv_msg->msg.data[0] != 0) {
		/*
		 * Got an error in the heartbeat response.  It was already
		 * reported in ipmi_wdog_msg_handler, but we should return
		 * an error here.
		 */
		rv = -EINVAL;
	}
	mutex_unlock(&priv->i2c_bus_lock);

out:
	kfree (i2c_ipmi_recv_msg);
	return rv;
}

static s32 i2c_ipmi_master_xfer (struct i2c_adapter *i2c_adap,
			struct i2c_msg *msgs, s32 num)
{
	struct i2c_ipmi_algo_data_t *adap = i2c_adap->algo_data;
	struct i2c_msg *pmsg;
	int error = 0;

	/* We support must 2 opertion only */
	if (num > 2)
		return -1;

	if (num == 1) {
		pmsg = &msgs[0];

		if (pmsg->flags & I2C_M_RD) {
			/* read */
			error = _i2c_ipmi_send_recv (&adap->ipmi,
					adap->bmc_bus, pmsg->addr << 1,
					pmsg->buf, pmsg->len,
					NULL, 0);
		} else {
			/* write */
			error = _i2c_ipmi_send_recv (&adap->ipmi,
					adap->bmc_bus, pmsg->addr << 1,
					NULL, 0,
					pmsg->buf, msgs->len);
		}
	}

	if (num == 2) {
		/* 1st is W */
		/* 2st is R */
			error = _i2c_ipmi_send_recv (&adap->ipmi,
					adap->bmc_bus, msgs[0].addr << 1,
					msgs[1].buf, msgs[1].len,
					msgs[0].buf, msgs[0].len);
	}

	if (error < 0) {
		return -EIO;
	}

	return num;
}


static u32 i2c_ipmi_bit_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL ;
}

static const struct i2c_algorithm i2c_ipmi_algo = {
	.master_xfer = i2c_ipmi_master_xfer,
	.functionality	= i2c_ipmi_bit_func,
};

static int __init i2c_ipmi_add_bus (struct i2c_adapter *adap)
{
	struct i2c_ipmi_algo_data_t *data = (struct i2c_ipmi_algo_data_t *)adap->algo_data;
	int rv = 0;

	if (i2c_ipmi_open (&data->ipmi))
		return -ENODEV;

	snprintf(adap->name, sizeof(adap->name), "bmc-i2c%d", data->bmc_bus);

	rv = i2c_add_adapter (adap);

	printk ("Mapping bmc-i2c%d to local bus %d\n", data->bmc_bus, adap->nr);
	return rv;
}

static int __init i2c_ipmi_init (void)
{
	int i;

	memset (&i2c_ipmi_adap,      0, sizeof(i2c_ipmi_adap));
	memset (&i2c_ipmi_adap_data, 0, sizeof(i2c_ipmi_adap_data));

	for (i = 0 ; i < I2C_IPMI_MAX_BUS ; i ++) {

		i2c_ipmi_adap[i].owner = THIS_MODULE;
		i2c_ipmi_adap[i].class = I2C_CLASS_HWMON | I2C_CLASS_SPD;

		i2c_ipmi_adap[i].algo      = &i2c_ipmi_algo;
		i2c_ipmi_adap[i].algo_data = &i2c_ipmi_adap_data[i];
	}

	// setup bmc bus id
	for (i = 0 ; i < bmc_bus_num ; i++) {
		i2c_ipmi_adap_data[i].bmc_bus = bmc_bus[i];
	}

	// add adapter
	for (i = 0 ; i < bmc_bus_num ; i++) {
		if (i2c_ipmi_add_bus(&i2c_ipmi_adap[i]) < 0)
			return -ENODEV;
	}

	return 0;
}

static void __exit i2c_ipmi_exit (void)
{
	int i;

	for (i = 0 ; i < bmc_bus_num; i ++) {

		i2c_del_adapter(&i2c_ipmi_adap[i]);

        i2c_ipmi_close(&i2c_ipmi_adap_data[i].ipmi);

	}
}



MODULE_AUTHOR("Dave Hu <dave.hu@deltaww.com>");
MODULE_DESCRIPTION("IPMI i2c bus");
MODULE_LICENSE("GPL");

module_init(i2c_ipmi_init);
module_exit(i2c_ipmi_exit);


