'''
    Arducam programable zoom-lens autofocus component.

    Copyright (c) 2019-4 Arducam <http://www.arducam.com>.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
    OR OTHER DEALINGS IN THE SOFTWARE.
'''

import sys
import time
import math
try:
    import cv2 #sudo apt-get install python-opencv
    import numpy as np
    # import picamera
    # from picamera.array import PiRGBArray
    from JetsonCamera import *
    from Focuser import Focuser 
except:
    sys.exit(0)

class AutoFocus:
    MAX_FOCUS_VALUE = 18000
    value_buffer = []
    focuser = None
    camera = None
    debug = False
    def __init__(self,focuser,camera):
        self.focuser = focuser
        self.camera = camera

    def get_end_point(self):
        end_point = self.focuser.end_point[int(math.floor(self.focuser.get(Focuser.OPT_ZOOM)/1000.0))]
        if self.debug:
            print("End Point: {}".format(end_point))
        return end_point
    def get_starting_point(self):
        starting_point = self.focuser.starting_point[int(math.ceil(self.focuser.get(Focuser.OPT_ZOOM)/1000.0))]
        if self.debug:
            print("Starting Point: {}".format(starting_point))
        return starting_point

    def filter(self,value):
        max_len = 3
        self.value_buffer.append(value)
        if len(self.value_buffer) == max_len:
            sort_list = sorted(self.value_buffer)
            self.value_buffer.pop(0)
            return sort_list[max_len / 2]
        return value

    def sobel(self,img):
        img_gray = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY)
        img_sobel = cv2.Sobel(img_gray,cv2.CV_16U,1,1)
        return cv2.mean(img_sobel)[0]

    def laplacian(self,img):
        img_gray = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY)
        img_sobel = cv2.Laplacian(img_gray,cv2.CV_16U)
        return cv2.mean(img_sobel)[0]
        
    def laplacian2(self,img):
        img_gray = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY) 
        img_sobel = cv2.Laplacian(img_gray,cv2.CV_64F).var()
        return img_sobel

    def calculation(self,camera):
        image = camera.getFrame()
        width = image.shape[1]
        height = image.shape[0]
        image = image[(height / 4):((height / 4) * 3),(width / 4):((width / 4) * 3)]
        #return laplacian(image)
        #return sobel(image)
        return self.laplacian2(image)

    def focusing(self,step,threshold,max_dec_count):
        self.value_buffer = []
        max_index = self.focuser.get(Focuser.OPT_FOCUS)
        max_value = 0.0
        last_value = -1
        dec_count = 0
        # step = 200
        focal_distance = max_index
        self.focuser.set(Focuser.OPT_FOCUS,focal_distance)
        while True:
            #Adjust focus
            self.focuser.set(Focuser.OPT_FOCUS,focal_distance)
            #Take image and calculate image clarity
            val = self.calculation(self.camera)
            # print "calculation value:",val
            val = self.filter(val)
            if self.debug:
                print("filter value = %d,focal_distance = %d"%(val,focal_distance))

            #Find the maximum image clarity
            if val > max_value:
                max_index = focal_distance
                max_value = val
                
            #If the image clarity starts to decrease
            if last_value - val > threshold :
                if self.debug:
                    print("dec-----last_value = %lf,current_value = %lf"%(last_value,val))
                dec_count += 1
            elif last_value - val != 0:
                dec_count = 0
            #Image clarity is reduced by six consecutive frames
            if dec_count > max_dec_count:
                break
            last_value = val

            #Increase the focal distance
            focal_distance = self.focuser.get(Focuser.OPT_FOCUS)
            focal_distance += step
            if focal_distance > self.MAX_FOCUS_VALUE:
                break

        return max_index,max_value

    def CoarseAdjustment(self,st_point,ed_point):
        images = []
        index_list = []
        eval_list = []
        time_list = []
        self.focuser.set(Focuser.OPT_FOCUS,st_point)

        image = self.camera.getFrame()
        time_list.append(time.time())
        images.append(image)

        # self.focuser.setFocusNoWait(self.focuser.end_point[int(self.focuser.getZoom()/1000)])
        self.focuser.set(Focuser.OPT_FOCUS,ed_point,0)
        while self.focuser.isBusy():
            image = self.camera.getFrame()
            time_list.append(time.time())
            images.append(image)
        '''
        frame = np.empty((self.camera.resolution.width * self.camera.resolution.height * 3,),dtype=np.uint8)
        for foo in self.camera.capture_continuous(frame,format='bgr',use_video_port=True):
            time_list.append(time.time())
            images.append(frame.copy().reshape(480,640,3))
            if not self.focuser.isBusy():
                break
        '''
        total_time = time_list[len(time_list) - 1] - time_list[0]
        index_list = np.arange(len(images))
        last_time = time_list[0]
        if self.debug:
            print("total images = %d"%(len(images)))
            print("total time = %d"%(total_time))
        for i in range(len(images)):
            image = images.pop(0)
            width = image.shape[1]
            height = image.shape[0]
            image = image[(height / 4):((height / 4) * 3),(width / 4):((width / 4) * 3)]
            result = self.laplacian2(image)
            eval_list.append(result)
        return eval_list,index_list,time_list

    def startFocus(self):
        begin = time.time()
        self.focuser.reset(Focuser.OPT_FOCUS)
        self.MAX_FOCUS_VALUE = self.get_end_point()
        self.focuser.set(Focuser.OPT_FOCUS,self.get_starting_point())
        if self.debug:
            print("init time = %lf"%(time.time() - begin))
        begin = time.time()
        max_index,max_value = self.focusing(300,1,1)
        # focuser.setFocus(0)
        self.focuser.set(Focuser.OPT_FOCUS,max_index - 300 * (2) - 30)
        # Careful adjustment
        max_index,max_value = self.focusing(50,1,4)
        self.focuser.set(Focuser.OPT_FOCUS,max_index - 30)
        if self.debug:
            print("focusing time = %lf"%(time.time() - begin))
        return max_index,max_value

    def startFocus2(self):
        begin = time.time()
        self.focuser.reset(Focuser.OPT_FOCUS)
        self.MAX_FOCUS_VALUE = self.get_end_point()
        starting_point = self.get_starting_point()

        if self.debug:
            print("init time = %lf"%(time.time() - begin))
        begin = time.time()
        eval_list,index_list,time_list = self.CoarseAdjustment(starting_point,self.MAX_FOCUS_VALUE)

        max_index = np.argmax(eval_list)
        total_time = time_list[len(time_list) - 1] - time_list[0]
        max_time = time_list[max_index - 1] - time_list[0]
        self.focuser.set(Focuser.OPT_FOCUS,int(((max_time - 0.0)/total_time)*(self.MAX_FOCUS_VALUE - starting_point)) + starting_point)
        # Careful adjustment
        max_index,max_value = self.focusing(50,1,4)
        self.focuser.set(Focuser.OPT_FOCUS,max_index - 30)
        if self.debug:
            print("focusing time = %lf"%(time.time() - begin))
        return max_index,max_value
    def auxiliaryFocusing(self):
        begin = time.time()
        # self.focuser.reset(Focuser.OPT_FOCUS)
        self.focuser.set(Focuser.OPT_FOCUS,0)
        # self.MAX_FOCUS_VALUE = self.focuser.end_point[int(self.focuser.get(Focuser.OPT_ZOOM)/1000)]
        # starting_point = self.focuser.starting_point[int(self.focuser.get(Focuser.OPT_ZOOM)/1000)]
        self.MAX_FOCUS_VALUE = 20000
        starting_point = 0
        if self.debug:
            print("init time = %lf"%(time.time() - begin))
        begin = time.time()
        
        eval_list,index_list,time_list = self.CoarseAdjustment(starting_point,self.MAX_FOCUS_VALUE)

        max_index = np.argmax(eval_list)
        total_time = time_list[len(time_list) - 1] - time_list[0]
        max_time = time_list[max_index] - time_list[0]
        self.focuser.set(Focuser.OPT_FOCUS,int(((max_time - 0.0)/total_time)*(self.MAX_FOCUS_VALUE - starting_point)) + starting_point)

        if self.debug:
            print("focusing time = %lf"%(time.time() - begin))
        return max_index        

pass


if __name__ == "__main__":
    camera = Camera()
    camera.start_preview()
    focuser = Focuser(1)
    autoFocus = AutoFocus(focuser, camera)
    autoFocus.debug = True
    # autoFocus.startFocus()
    autoFocus.startFocus2()
    time.sleep(5)
    camera.stop_preview()
    camera.close()
