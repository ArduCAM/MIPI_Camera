'''
    Arducam programable zoom-lens autofocus example.

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

import cv2 #sudo apt-get install python-opencv
import numpy as py
import os
import time

from JetsonCamera import Camera
from Focuser import Focuser

from AutoFocus import AutoFocus

if __name__ == "__main__":
    #open camera
    camera = Camera()
    #open camera preview
    camera.start_preview()

    focuser = Focuser(1)
    print("Focus value = %d\n" % focuser.get(Focuser.OPT_FOCUS))
    auto_focus = AutoFocus(focuser,camera)
    auto_focus.debug = True
    begin = time.time()
    max_index,max_value = auto_focus.startFocus()
    # max_index,max_value = auto_focus.startFocus2()
    print("total time = %lf"%(time.time() - begin))
    #Adjust focus to the best
    time.sleep(5)
    print("max index = %d,max value = %lf" % (max_index,max_value))
        
    camera.stop_preview()
    camera.close()

    
