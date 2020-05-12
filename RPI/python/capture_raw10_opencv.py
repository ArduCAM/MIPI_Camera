import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2
import time
import numpy as np
import cv2 #sudo apt-get install python-opencv
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
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        camera.set_mode(6) # chose a camera mode which yields raw10 pixel format, see output of list_format utility
        fmt = camera.get_format()
        width = fmt.get("width")
        height = fmt.get("height")
        print("Current resolution is {w}x{h}".format(w=width, h=height))
        # print("Start preview...")
        # camera.start_preview(fullscreen = False, window = (0, 0, 1280, 720))
        set_controls(camera)
        time.sleep(1)
        while cv2.waitKey(10) != 27:
            frame = camera.capture(encoding = 'raw')
            #height = fmt[1]
            #width  = fmt[0]
            frame = arducam.remove_padding(frame.data, width, height, 10)
            frame = arducam.unpack_mipi_raw10(frame)
            frame = frame.reshape(height, width) << 6
            image = frame
            #image = cv2.cvtColor(frame, cv2.COLOR_YUV2BGR_I420)
            cv2.imshow("Arducam", image)

        # Release memory
        del frame
        # print("Stop preview...")
        # camera.stop_preview()
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)
