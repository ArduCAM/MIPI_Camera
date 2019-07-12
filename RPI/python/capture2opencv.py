import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2
import time
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
        print("Setting the resolution...")
        fmt = camera.set_resolution(1920, 1080)
        print("Current resolution is {}".format(fmt))
        set_controls(camera)
        while cv2.waitKey(10) != 27:
            frame = camera.capture(encoding = 'i420')
            height = int(align_up(fmt[1], 16))
            width = int(align_up(fmt[0], 32))
            image = frame.as_array.reshape(int(height * 1.5), width)
            image = cv2.cvtColor(image, cv2.COLOR_YUV2BGR_I420)
            cv2.imshow("Arducam", image)


        # Release memory
        del frame
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)
