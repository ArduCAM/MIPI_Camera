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

import os
import time
from datetime import datetime
from arducam_camera import MyCamera
import json
import cv2
import numpy as np

try:
  camera_params = json.load(open("camera_params.txt", "r"))
except Exception as e:
  print(e)
  print("Please run 1_test.py first.")
  exit(-1)


# Photo session settings
total_photos = 30             # Number of images to take
countdown = 5                 # Interval for count-down timer, seconds
font=cv2.FONT_HERSHEY_SIMPLEX # Cowntdown timer font

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


# Lets start taking photos! 
counter = 0
t2 = datetime.now()
print ("Starting photo sequence")
while True:
    frame = camera.get_frame()
    frame = cv2.resize(frame, (img_width, img_height))
    t1 = datetime.now()
    cntdwn_timer = countdown - int ((t1-t2).total_seconds())
    # If cowntdown is zero - let's record next image
    if cntdwn_timer == -1:
      counter += 1
      filename = './scenes/scene_'+str(img_width)+'x'+str(img_height)+'_'+\
                  str(counter) + '.png'
      cv2.imwrite(filename, frame)
      print (' ['+str(counter)+' of '+str(total_photos)+'] '+filename)
      t2 = datetime.now()
      time.sleep(1)
      cntdwn_timer = 0      # To avoid "-1" timer display 
      next
    # Draw cowntdown counter, seconds
    cv2.putText(frame, str(cntdwn_timer), (50,50), font, 2.0, (0,0,255),4, cv2.LINE_AA)
    cv2.imshow("pair", frame)
    key = cv2.waitKey(1) & 0xFF
    
    # Press 'Q' key to quit, or wait till all photos are taken
    if (key == ord("q")) | (counter == total_photos):
      break
camera.close_camera()
 
print ("Photo sequence finished")
 
