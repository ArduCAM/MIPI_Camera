import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2
import time
import cv2 #sudo apt-get install python-opencv
import RPi.GPIO as gp
import sys
import os

gp.setwarnings(False)
gp.setmode(gp.BOARD)

gp.setup(7, gp.OUT)
gp.setup(11, gp.OUT)
gp.setup(12, gp.OUT)

camera_info = {
    "A" : {
        "i2c" : "i2cset -y 1 0x70 0x00 0x04",
        "gpio": [0, 0, 1],
    },
    "B" : {
        "i2c" : "i2cset -y 1 0x70 0x00 0x05",
        "gpio": [1, 0, 1],
    },
    "C" : {
        "i2c" : "i2cset -y 1 0x70 0x00 0x06",
        "gpio": [0, 1, 0],
    },
    "D" : {
        "i2c" : "i2cset -y 1 0x70 0x00 0x07",
        "gpio": [1, 1, 0],
    },
}

def switch_camera(index):
    info = camera_info.get(index)
    if info == None:
        raise TypeError("Unknown parameter.")
    os.system(info["i2c"])
    gpio_info = info["gpio"]
    gp.output(7, gpio_info[0])
    gp.output(11, gpio_info[1])
    gp.output(12, gpio_info[2])

def init_all_camera(mipi_camera, width, height):
    for i in range(4):
        switch_camera(chr(65 + i))
        fmt = camera.set_resolution(width, height)
    return fmt

def align_down(size, align):
    return (size & ~((align)-1))

def align_up(size, align):
    return align_down(size + align - 1, align)

def set_controls(camera):
    try:
        print("Reset the focus...")
        camera.reset_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")

    try:
        print("Enable Auto Exposure...")
        camera.software_auto_exposure(enable = True)
        print("Enable Auto White Balance...")
        camera.software_auto_white_balance(enable = True)
    except Exception as e:
        print(e)


if __name__ == "__main__":
    try:
        # Open the first channel of I2c 
        switch_camera('A')
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        print("Setting the resolution...")
        fmt = init_all_camera(camera, 1920, 1080)
        # set_controls(camera)
        index = ord('A')
        # switch to A channel
        switch_camera(chr(index))
        while cv2.waitKey(10) != 27:
            frame = camera.capture(encoding = 'i420')
            switch_camera(chr(index))
            height = int(align_up(fmt[1], 16))
            width = int(align_up(fmt[0], 32))
            image = frame.as_array.reshape(int(height * 1.5), width)
            image = cv2.cvtColor(image, cv2.COLOR_YUV2BGR_I420)
            image = cv2.resize(image, (640, 360))
            cv2.imshow("Arducam {}".format(chr(index)), image)
            index += 1
            if index > ord('D'):
                index = ord('A')

        # Release memory
        del frame
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)
