from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_dsc100_r0(OnlPlatformDelta,OnlPlatformPortConfig_16x100):

    PLATFORM='x86-64-delta-dsc100-r0'
    MODEL="DSC100"
    SYS_OBJECT_ID=".100"

    def baseconfig(self):
        self.new_i2c_device('pca9544', 0x70, 2);

        self.insmod('x86-64-delta-dsc100-i2c-ipmi.ko', params={'bmc_bus':'0,4,5'})
        self.insmod('x86-64-delta-dsc100-i2c-mux-setting.ko')
        self.insmod('x86-64-delta-dsc100-i2c-mux-cpld.ko')
        os.system('/usr/lib/python2.7/dist-packages/onl/platform/x86_64_delta_dsc100_r0/LC_FC_network_cfg.sh') 
        ########### initialize I2C bus 12 ###########

        
        self.new_i2c_devices(
            [
                ('tmp75', 0x4f, 12),
                ('tmp75', 0x4d, 12),
                ('tmp75', 0x4c, 12),
                ('tmp75', 0x4a, 12),
            ]
            )

    
        return True
