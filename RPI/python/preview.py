import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2
import time

def set_controls(camera):
    try:
        print("Reset the focus...")
        camera.reset_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")
    try:
        time.sleep(2)
        print("Setting the exposure...")
        camera.set_control(v4l2.V4L2_CID_EXPOSURE, 10)
        time.sleep(2)
        print("Setting the exposure...")
        camera.set_control(v4l2.V4L2_CID_EXPOSURE, 3000)
        time.sleep(2)
        print("Setting the hfilp...")
        camera.set_control(v4l2.V4L2_CID_HFLIP, 1)
        time.sleep(2)
        print("Enable Auto Exposure...")
        camera.software_auto_exposure(enable = True)
        time.sleep(2)
        print("Enable Auto White Balance...")
        camera.software_auto_white_balance(enable = True)
        print("manual set awb compensation...")
        camera.manual_set_awb_compensation(100,100)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")

if __name__ == "__main__":
    try:
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        print("Setting the resolution...")
        fmt = camera.set_resolution(1920, 1080)
        print("Current resolution is {}".format(fmt))
        print("Start preview...")
        camera.start_preview(fullscreen = False, window = (0, 0, 1280, 720))
        set_controls(camera)
        time.sleep(10)
        print("Setting the resolution...")
        fmt = camera.set_resolution(3280, 2464)
        print("Current resolution is {}".format(fmt))
        time.sleep(10)
        print("Stop preview...")
        camera.stop_preview()
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)
