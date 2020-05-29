import sys
sys.path.append('../')
import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2
import time
import numpy as np
import cv2
from isp_lib import *

def set_controls(camera):
    try:
        print("Reset the focus...")
        camera.reset_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")

    # try:
    #     print("Enable Auto Exposure...")
    #     camera.software_auto_exposure(enable = True)
    #     print("Enable Auto White Balance...")
    #     camera.software_auto_white_balance(enable = True)
    # except Exception as e:
    #     print(e)

def resize(frame, dst_width=640):
    height = frame.shape[0]
    width = frame.shape[1]

    scale = (dst_width * 1.0) / width
    return cv2.resize(frame, (int(scale * width), int(scale * height))) 


if __name__ == "__main__":
    try:
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        _isp = isp(camera.camera_instance)
        print("Setting the mode...")
        camera.set_mode(0)
        fmt = camera.get_format()
        fmt = (fmt["width"], fmt["height"])
        print("Current resolution is {}".format(fmt))
        set_controls(camera)

        start_time = time.time()
        do_change = True
        while True:
            data = camera.capture(encoding = 'raw')
            # Use different variable names to avoid memory being released
            frame = arducam.unpack_raw10_to_raw8(data.buffer_ptr, fmt[0], fmt[1])
            frame = cv2.cvtColor(frame.as_array.reshape((fmt[1], fmt[0])), cv2.COLOR_BAYER_RG2BGR)
            _isp.run_awb(frame)
            _isp.run_ae(frame)

            disp = resize(frame)

            cv2.imshow("Arducam", disp)
            ret = cv2.waitKey(10)
            if ret == ord('q'):
                break

           # if time.time() - start_time >= 5 and do_change:
           #     do_change = False
           #     camera.set_mode(2)
           #     fmt = camera.get_format()
           #     fmt = (fmt["width"], fmt["height"])
           #     start_time = time.time()
           # if time.time() - start_time >= 5 and not do_change:
           #     cv2.imwrite("Test.jpg", frame)
           #     break

        # Release memory
        del frame
        del data
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)
