import cv2
import numpy as np
from datetime import datetime
import array
import fcntl
import os
import argparse
import time
from utils import ArducamUtils

def resize(frame, dst_width):
    width = frame.shape[1]
    height = frame.shape[0]
    scale = dst_width / width
    return cv2.resize(frame, (int(scale * width), int(scale * height)))

def display(cap, arducam_utils, device_num):
    counter = 0
    frame_count = 0
    start_time = datetime.now()
    cv2.namedWindow("Arducam")
    start = time.time()
    while True:
        ret, frame = cap.read()
        if ret:
            counter += 1

            frame = arducam_utils.convert(frame)
            
            frame = resize(frame, 640.0)
            # frame = cv2.rotate(frame, cv2.ROTATE_180)
            # display
            cv2.imshow("Arducam", frame)
            frame_count += 1
        ret = cv2.waitKey(10)
        # press 'q' to exit.
        if ret == ord('q'):
            break

        if time.time() - start >= 1:
            print("fps: {}".format(frame_count))
            start = time.time()
            frame_count = 0 

    end_time = datetime.now()
    elapsed_time = end_time - start_time
    avgtime = elapsed_time.total_seconds() / counter
    print ("Average time between frames: " + str(avgtime))
    print ("Average FPS: " + str(1/avgtime))


def fourcc(a, b, c, d):
    return ord(a) | (ord(b) << 8) | (ord(c) << 16) | (ord(d) << 24)

def pixelformat(string):
    if len(string) != 3 and len(string) != 4:
        msg = "{} is not a pixel format".format(string)
        raise argparse.ArgumentTypeError(msg)
    if len(string) == 3:
        return fourcc(string[0], string[1], string[2], ' ')
    else:
        return fourcc(string[0], string[1], string[2], string[3])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Arducam Jetson Nano MIPI Camera Sensor.')

    parser.add_argument('-d', '--device', default=0, type=int, nargs='?',
                        help='/dev/videoX default is 0')
    parser.add_argument('-f', '--pixelformat', type=pixelformat,
                        help="set pixelformat")
    parser.add_argument('--width', type=lambda x: int(x,0),
                        help="set width of image")
    parser.add_argument('--height', type=lambda x: int(x,0),
                        help="set height of image")

    args = parser.parse_args()

    # open camera
    cap = cv2.VideoCapture(args.device, cv2.CAP_V4L2)
    # set pixel format
    # cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('Y', '1', '6', ' '))

    if args.pixelformat != None:
        if not cap.set(cv2.CAP_PROP_FOURCC, args.pixelformat):
            print("Failed to set pixel format.")

    arducam_utils = ArducamUtils(args.device)
    # turn off RGB conversion
    if arducam_utils.convert2rgb == 0:
        cap.set(cv2.CAP_PROP_CONVERT_RGB, arducam_utils.convert2rgb)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 4)
    # set width
    if args.width != None:
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, args.width)
    # set height
    if args.height != None:
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, args.height)
    cap.grab()
    time.sleep(1)
    os.system("v4l2-ctl -d {} -c trigger_mode=1".format(args.device))
    os.system("v4l2-ctl -d {} -c frame_timeout=10000".format(args.device))
    cap.grab()

    # After the above operation, there are still two frames of data in the buffer, 
    # and one frame can be read out every time one frame is triggered.Therefore, 
    # there is a two-frame delay from triggering to reading the image.
    #　The first two frames are not the triggered image.

    # begin display
    display(cap, arducam_utils, args.device)

    # release camera
    cap.release()
