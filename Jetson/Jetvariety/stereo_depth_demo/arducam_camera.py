import cv2 #sudo apt-get install python-opencv
import os
import numpy as np
from utils import ArducamUtils


class MyCamera(object):
    def __init__(self):
        pass
    def open_camera(self, device = 0, width = -1, height = -1):
        self.cap = cv2.VideoCapture(device, cv2.CAP_V4L2)
        self.arducam_utils = ArducamUtils(device)
        # turn off RGB conversion
        if self.arducam_utils.convert2rgb == 0:
            self.cap.set(cv2.CAP_PROP_CONVERT_RGB, self.arducam_utils.convert2rgb)
        # set width
        if width != -1:
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        # set height
        if height != -1:
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
        
        self.fix_orientation()

    def fix_orientation(self):
        _, sensor_id = self.arducam_utils.read_dev(ArducamUtils.FIRMWARE_SENSOR_ID_REG)
        print("0x{:X}".format(sensor_id))
        if sensor_id == 0x5647:
            self.arducam_utils.cvt_code = cv2.COLOR_BAYER_GB2BGR
            self.cap.grab()
            self.arducam_utils.write_sensor(0x3820, 0x42)
            self.arducam_utils.write_sensor(0x3821, 0x00)
        elif sensor_id == 0x0219:
            self.arducam_utils.cvt_code = cv2.COLOR_BAYER_RG2BGR
            self.cap.grab()
            self.arducam_utils.write_sensor(0x0172, 0x03)

    def get_framesize(self):
        width = self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        height = self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        return (width, height)

    def close_camera(self):
        self.cap.release()
        self.cap = None

    def get_frame(self):
        ret, frame = self.cap.read()
        if not ret:
            return None
        return self.arducam_utils.convert(frame)
