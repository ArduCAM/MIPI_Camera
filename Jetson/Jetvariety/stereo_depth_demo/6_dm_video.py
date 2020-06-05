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
from arducam_camera import MyCamera
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

 # Initialize the camera
camera = MyCamera()

# Camera settimgs
cam_width = camera_params['width']    # Cam sensor width settings
cam_height = camera_params['height']  # Cam sensor height settings
print ("Used camera resolution: "+str(cam_width)+" x "+str(cam_height))

print("Open camera...")
camera.open_camera(camera_params['device'], cam_width, cam_height)
fmt = camera.get_framesize()
print("Current resolution: {}x{}".format(fmt[0], fmt[1]))

scale = camera_params['scale']
# Buffer for captured image settings
img_width = int(camera_params['width'] * scale)
img_height = int(camera_params['height'] * scale)
print ("Scaled image resolution: "+str(img_width)+" x "+str(img_height))

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
    frame = camera.get_frame()
    frame = cv2.resize(frame, (img_width, img_height))
    t1 = datetime.now()
    # pair_img = frame
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

camera.close_camera()
