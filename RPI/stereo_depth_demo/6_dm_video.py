# Copyright (C) 2019 Eugene Pomazov, <stereopi.com>, virt2real team
#
# This file is part of StereoPi tutorial scripts.
#
# StereoPi tutorial is free software: you can redistribute it 
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the 
# License, or (at your option) any later version.
#
# StereoPi tutorial is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with StereoPi tutorial.  
# If not, see <http://www.gnu.org/licenses/>.
#
# Most of this code is updated version of 3dberry.org project by virt2real
# 
# Thanks to Adrian and http://pyimagesearch.com, as there are lot of
# code in this tutorial was taken from his lessons.
# 


import time
import cv2
import numpy as np
import json
from stereovision.calibration import StereoCalibrator
from stereovision.calibration import StereoCalibration
from datetime import datetime
import arducam_mipicamera as arducam
# Depth map default preset
SWS = 5
PFS = 5
PFC = 29
MDS = -30
NOD = 160
TTH = 100
UR = 10
SR = 14
SPWS = 100

try:
  camera_params = json.load(open("camera_params.txt", "r"))
except Exception as e:
  print(e)
  print("Please run 1_test.py first.")
  exit(-1)

def align_down(size, align):
    return (size & ~((align)-1))

def align_up(size, align):
    return align_down(size + align - 1, align)

def get_frame(camera):
    frame = camera.capture(encoding = 'i420')
    fmt = camera.get_format()
    height = int(align_up(fmt['height'], 16))
    width = int(align_up(fmt['width'], 32))
    image = frame.as_array.reshape(int(height * 1.5), width)
    image = cv2.cvtColor(image, cv2.COLOR_YUV2BGR_I420)
    image = image[:fmt['height'], :fmt['width']]
    return image

# Initialize the camera
camera = arducam.mipi_camera()
print("Open camera...")
camera.init_camera()
mode = camera_params['mode']
camera.set_mode(mode)
fmt = camera.get_format()
print("Current mode: {},resolution: {}x{}".format(fmt['mode'], fmt['width'], fmt['height']))

# Camera settimgs
cam_width = fmt['width']
cam_height = fmt['height']

img_width= camera_params['width']
img_height = camera_params['height']
print ("Scaled image resolution: "+str(img_width)+" x "+str(img_height))

# camera.set_control(0x00980911, 1000)
# Implementing calibration data
print('Read calibration data and rectifying stereo pair...')
calibration = StereoCalibration(input_folder='calib_result')

# Initialize interface windows
cv2.namedWindow("Image")
cv2.moveWindow("Image", 50,100)
cv2.namedWindow("left")
cv2.moveWindow("left", 450,100)
cv2.namedWindow("right")
cv2.moveWindow("right", 850,100)


disparity = np.zeros((img_width, img_height), np.uint8)
sbm = cv2.StereoBM_create(numDisparities=0, blockSize=21)

def stereo_depth_map(rectified_pair):
    dmLeft = rectified_pair[0]
    dmRight = rectified_pair[1]
    disparity = sbm.compute(dmLeft, dmRight)
    local_max = disparity.max()
    local_min = disparity.min()
    disparity_grayscale = (disparity-local_min)*(65535.0/(local_max-local_min))
    disparity_fixtype = cv2.convertScaleAbs(disparity_grayscale, alpha=(255.0/65535.0))
    disparity_color = cv2.applyColorMap(disparity_fixtype, cv2.COLORMAP_JET)
    cv2.imshow("Image", disparity_color)
    key = cv2.waitKey(1) & 0xFF   
    if key == ord("q"):
        quit();
    return disparity_color

def load_map_settings( fName ):
    global SWS, PFS, PFC, MDS, NOD, TTH, UR, SR, SPWS, loading_settings
    print('Loading parameters from file...')
    f=open(fName, 'r')
    data = json.load(f)
    SWS=data['SADWindowSize']
    PFS=data['preFilterSize']
    PFC=data['preFilterCap']
    MDS=data['minDisparity']
    NOD=data['numberOfDisparities']
    TTH=data['textureThreshold']
    UR=data['uniquenessRatio']
    SR=data['speckleRange']
    SPWS=data['speckleWindowSize']    
    #sbm.setSADWindowSize(SWS)
    sbm.setPreFilterType(1)
    sbm.setPreFilterSize(PFS)
    sbm.setPreFilterCap(PFC)
    sbm.setMinDisparity(MDS)
    sbm.setNumDisparities(NOD)
    sbm.setTextureThreshold(TTH)
    sbm.setUniquenessRatio(UR)
    sbm.setSpeckleRange(SR)
    sbm.setSpeckleWindowSize(SPWS)
    f.close()
    print ('Parameters loaded from file '+fName)


load_map_settings ("3dmap_set.txt")

# capture frames from the camera
# for frame in camera.capture_continuous(capture, format="bgra", use_video_port=True, resize=(img_width,img_height)):
while True:
    frame = get_frame(camera)
    frame = cv2.resize(frame, (img_width, img_height))

    t1 = datetime.now()
    pair_img = cv2.cvtColor (frame, cv2.COLOR_BGR2GRAY)
    imgLeft = pair_img [0:img_height,0:int(img_width/2)] #Y+H and X+W
    imgRight = pair_img [0:img_height,int(img_width/2):img_width] #Y+H and X+W
    rectified_pair = calibration.rectify((imgLeft, imgRight))
    disparity = stereo_depth_map(rectified_pair)
    # show the frame
    cv2.imshow("left", imgLeft)
    cv2.imshow("right", imgRight)    

    t2 = datetime.now()
    print ("DM build time: " + str(t2-t1))


