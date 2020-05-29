from ctypes import *
import numpy as np
import gc
_dll = CDLL("./libisp_lib.so")

class isp(object):
    def __init__(self, camera_instance):
        self.camera_instance = camera_instance
        self.instance = c_void_p(0)
        _dll.create_isp(byref(self.instance), self.camera_instance)
    
    def run_awb(self, img):
        w = img.shape[1]
        h = img.shape[0]
        # https://github.com/numpy/numpy/blob/1c58504eec43f9ba18ac835131fed496fb59772d/numpy/core/_internal.py#L266
        # _dll.run_auto_white_balance(self.instance, c_void_p(img.ctypes.data), w, h)
        _dll.run_auto_white_balance(self.instance, img.ctypes.data_as(c_void_p), w, h)
        gc.collect()

    def run_ae(self, img):
        w = img.shape[1]
        h = img.shape[0]
        _dll.run_auto_exposure(self.instance, img.ctypes.data_as(c_void_p), w, h)
        gc.collect()
