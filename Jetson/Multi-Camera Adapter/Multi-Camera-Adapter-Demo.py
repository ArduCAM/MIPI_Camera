import RPi.GPIO as gp # sudo pip install Jetson.GPIO #https://pypi.org/project/Jetson.GPIO/#description
import os
import argparse

def parse_cmdline():
    parser = argparse.ArgumentParser(description='Arducam Controller.')

    parser.add_argument('-i', '--i2c-bus', type=int, nargs=None, required=True,
                        help='Set i2c bus, for Jetson Nano it is 1, for Jetson Xavier NX it is 8.')

    return parser.parse_args()

gp.setwarnings(False)
gp.setmode(gp.BOARD)

gp.setup(7, gp.OUT)
gp.setup(11, gp.OUT)
gp.setup(12, gp.OUT)

def main(i2c_bus):
    print("Start testing the camera A")
    i2c = "i2cset -y {} 0x70 0x00 0x04".format(i2c_bus)
    os.system(i2c)
    gp.output(7, False)
    gp.output(11, False)
    gp.output(12, True)
    capture(1)
    print("Start testing the camera B")
    i2c = "i2cset -y {} 0x70 0x00 0x05".format(i2c_bus)
    os.system(i2c)
    gp.output(7, True)
    gp.output(11, False)
    gp.output(12, True)
    capture(2)
    print("Start testing the camera C")
    i2c = "i2cset -y {} 0x70 0x00 0x06".format(i2c_bus)
    os.system(i2c)
    gp.output(7, False)
    gp.output(11, True)
    gp.output(12, False)
    capture(3)
    print("Start testing the camera D")
    i2c = "i2cset -y {} 0x70 0x00 0x07".format(i2c_bus)
    os.system(i2c)
    gp.output(7, True)
    gp.output(11, True)
    gp.output(12, False)
    capture(4)

def capture(cam):
    #cmd = "raspistill -o capture_%d.jpg" % cam
    cmd = "nvgstcapture-1.0 -A --capture-auto -S 0 --image-res=3 --file-name=capture_%d.jpg" % cam
    os.system(cmd)

if __name__ == "__main__":
    args = parse_cmdline()
    print(args.i2c_bus)
    main(args.i2c_bus)

    gp.output(7, False)
    gp.output(11, False)
    gp.output(12, True)
