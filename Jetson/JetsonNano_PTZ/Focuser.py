'''
    Arducam programable zoom-lens control component.

    Copyright (c) 2019-4 Arducam <http://www.arducam.com>.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
    OR OTHER DEALINGS IN THE SOFTWARE.
'''

import sys
import time
class Focuser:
    bus = None
    CHIP_I2C_ADDR = 0x0C
    BUSY_REG_ADDR = 0x04

    # starting_point = [
    #     17320, 13870, 10470,
    #     7820, 5470, 3720,
    #     2320, 1220, 370,
    #     0, 0, 0, 0,
    #     0, 0, 0, 0,
    #     0, 0, 0, 0
    # ]
    
    # end_point = [
    #     20000, 20000, 17370,
    #     14470, 12170, 10270,
    #     8770, 7670, 6820,
    #     5970, 5520, 5220,
    #     4970, 4820, 4770,
    #     4820, 4870, 5020,
    #     5270, 5520, 5770
    # ]
    starting_point =     [
        10000, 10000, 10000, 
        10720, 8070, 5970, 
        4320, 2920, 1920, 
        970, 520, 20, 0, 
        0, 0, 0, 0, 0, 0, 
        0, 0
    ]

    end_point = [
        20000, 20000, 20000, 
        20000, 19620, 17020, 
        14920, 13170, 12020, 
        10970, 10170, 9770, 
        9170, 9020, 8820, 
        8570, 8570, 8570, 
        8770, 8970, 9170
    ]


    def __init__(self, bus):
        try:
            import smbus # sudo apt-get install python-smbus
            self.bus = smbus.SMBus(bus)
        except:
            sys.exit(0)
        
    def read(self,chip_addr,reg_addr):
        value = self.bus.read_word_data(chip_addr,reg_addr)
        value = ((value & 0x00FF)<< 8) | ((value & 0xFF00) >> 8)
        return value
    def write(self,chip_addr,reg_addr,value):
        if value < 0:
            value = 0
        value = ((value & 0x00FF)<< 8) | ((value & 0xFF00) >> 8)
        return self.bus.write_word_data(chip_addr,reg_addr,value)
    def isBusy(self):
        return self.read(self.CHIP_I2C_ADDR,self.BUSY_REG_ADDR) != 0
    def waitingForFree(self):
        count = 0
        begin = time.time()
        while self.isBusy() and count < (5 / 0.01):
            count += 1
            time.sleep(0.01)
        # if count >= (5 / 0.01):
        #     print "wait timeout."
        # elif count != 0:
        #     print "wait time = %lf"%(time.time() - begin)

    OPT_BASE    = 0x1000
    OPT_FOCUS   = OPT_BASE | 0x01
    OPT_ZOOM    = OPT_BASE | 0x02
    OPT_MOTOR_X = OPT_BASE | 0x03
    OPT_MOTOR_Y = OPT_BASE | 0x04
    OPT_IRCUT   = OPT_BASE | 0x05
    opts = {
        OPT_FOCUS : {
            "REG_ADDR" : 0x01,
            "MIN_VALUE": 0,
            "MAX_VALUE": 20000,
            "RESET_ADDR": 0x01 + 0x0A,
        },
        OPT_ZOOM  : {
            "REG_ADDR" : 0x00,
            "MIN_VALUE": 3000,
            "MAX_VALUE": 20000,
            "RESET_ADDR": 0x00 + 0x0A,
        },
        OPT_MOTOR_X : {
            "REG_ADDR" : 0x05,
            "MIN_VALUE": 0,
            "MAX_VALUE": 180,
            "RESET_ADDR": None,
        },
        OPT_MOTOR_Y : {
            "REG_ADDR" : 0x06,
            "MIN_VALUE": 0,
            "MAX_VALUE": 180,
            "RESET_ADDR": None,
        },
        OPT_IRCUT : {
            "REG_ADDR" : 0x0C, 
            "MIN_VALUE": 0x00,
            "MAX_VALUE": 0x01,   #0x0001 open, 0x0000 close
            "RESET_ADDR": None,
        }
    }
    def reset(self,opt,flag = 1):
        self.waitingForFree()
        info = self.opts[opt]
        if info == None or info["RESET_ADDR"] == None:
            return
        self.write(self.CHIP_I2C_ADDR,info["RESET_ADDR"],0x0000)
        self.set(opt,info["MIN_VALUE"])
        if flag & 0x01 != 0:
            self.waitingForFree()

    def get(self,opt,flag = 0):
        self.waitingForFree()
        info = self.opts[opt]
        return self.read(self.CHIP_I2C_ADDR,info["REG_ADDR"])

    def set(self,opt,value,flag = 1):
        self.waitingForFree()
        info = self.opts[opt]
        if value > info["MAX_VALUE"]:
            value = info["MAX_VALUE"]
        elif value < info["MIN_VALUE"]:
            value = info["MIN_VALUE"]
        self.write(self.CHIP_I2C_ADDR,info["REG_ADDR"],value)
        if flag & 0x01 != 0:
            self.waitingForFree()

pass 


def test():
    focuser = Focuser(1)
    focuser.reset(Focuser.OPT_FOCUS)
    while focuser.get(Focuser.OPT_FOCUS) < 18000:
        focuser.set(Focuser.OPT_FOCUS,focuser.get(Focuser.OPT_FOCUS) + 50)
    focuser.set(Focuser.OPT_FOCUS,0)
    focuser.set(Focuser.OPT_FOCUS,10000)
pass

if __name__ == "__main__":
    test()
