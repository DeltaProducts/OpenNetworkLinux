#!/bin/sh


############################################################


#############################################################
# Setup bond, vlan...
#############################################################

#######
## Rename the real ethx to real_ethx
#######
ip link set dev ma1 down
ip link set dev ma1 name real_eth0
ip link set dev eth1 name real_eth1

#######
## Create Bond, set to active-backup mode, and up.
#######
echo +trunk0 > /sys/class/net/bonding_masters
sleep 1
echo 1       > /sys/devices/virtual/net/trunk0/bonding/mode
echo 100     > /sys/devices/virtual/net/trunk0/bonding/miimon
echo failure > /sys/devices/virtual/net/trunk0/bonding/primary_reselect

echo +real_eth0 > /sys/devices/virtual/net/trunk0/bonding/slaves
echo +real_eth1 > /sys/devices/virtual/net/trunk0/bonding/slaves
ip link set dev trunk0 up

#######
## eth0 for external access.
#######
ip link add link trunk0 name ma1 type vlan id 1
ip link set dev ma1  up

#######
## eth1 for customer backup internal work between MC/LC/FC
## setup ip address according to slot id
#######
ip link add link trunk0 name eth1 type vlan id 99
ip link set dev eth1 up

#############################################################
##config the dev ip based on the board slot id
#############################################################

slot_id=`i2cget -y 7 0x31 0x00`
slot_id=`printf %d $slot_id`
slot_id=$(($slot_id >> 4))

ifconfig eth1 169.254.0.$slot_id

#################################
echo "Completing bootup event..."

echo "0x04 0x1f 0x00 0x6f 0x06 0xff 0xff" > /tmp/sel_bootcomplete && {
   ipmitool sel add /tmp/sel_bootcomplete 2>/dev/null 1>/dev/null
}

#############################################################
##send the onl os information to bmc
#############################################################
# OS Information
if [ -e /etc/onl/rootfs/manifest.json ]
then
#   . /etc/onl/rootfs/manifest.json
    PRODUCT_VERSION=`grep -w PRODUCT_VERSION /etc/onl/rootfs/manifest.json | awk -F \" '{print $4}'`
    BUILD_TIMESTAMP=`grep -w BUILD_TIMESTAMP /etc/onl/rootfs/manifest.json | awk -F \" '{print $4}'`
    OS_VER="$PRODUCT_VERSION"
    OS_BUD="$BUILD_TIMESTAMP"
elif [ -e /etc/onl/loader/manifest.json ]
then
    #   . /etc/onl/loader/manifest.json
    PRODUCT_VERSION=`grep -w PRODUCT_VERSION /etc/onl/loader/manifest.json | awk -F \" '{print $4}'`
    BUILD_TIMESTAMP=`grep -w BUILD_TIMESTAMP /etc/onl/loader/manifest.json | awk -F \" '{print $4}'`
    OS_VER="$PRODUCT_VERSION"
    OS_BUD="$BUILD_TIMESTAMP"
else
    OS_VER="ONL-0.0.0"
    OS_BUD="0-0-0"
fi
OS_STR=`echo -n "OS=$OS_VER $OS_BUD" | sed 's/ /-/g'`

# CPLD Information

#LC/FC cpu cpld
CPLD_STR=""
val=`i2cget -y 6 0x30 0x01 2>/dev/null`
if [ ! -z "$val" ]
then
    CPLD_STR="$CPLD_STR PLD1=$val"
fi

#MC cpld version
val=`i2cget -y 6 0x31 0x00 2>/dev/null`
if [ ! -z "$val" ]
then
    CPLD_STR="$CPLD_STR PLD2=$val"
fi

#LC/FC SWPLD
val=`i2cget -y 7 0x31 0x3f 2>/dev/null`
if [ ! -z "$val" ]
then
    CPLD_STR="$CPLD_STR PLD3=$val"
fi

BMC_STR="$BIOS_STR $CPLD_STR"

# If there is not PLD information, skip
have_pld=`echo $BMC_STR | grep PLD`
if [ ! -z "$have_pld" ]
then
    ipmitool mc setsysinfo system_fw_version "$BMC_STR" 2>/dev/null
fi

ipmitool mc setsysinfo primary_os_name "$OS_STR" 2>/dev/null
ipmitool mc setsysinfo os_name "$OS_STR" 2>/dev/null

############################################################
echo "Setting system led..."

i2cset -y 7 0x31 0x0a 0x19

echo "Disable watchdog..."

ipmitool raw 0x30 0x57 0x00 0x00


