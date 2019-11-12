# encoding: UTF-8
'''
    Arducam programable zoom-lens controller.

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
import sys
import time
from JetsonCamera import Camera

from Focuser import Focuser
from AutoFocus import AutoFocus
import curses

global image_count
image_count = 0

# Rendering status bar
def RenderStatusBar(stdscr):
    height, width = stdscr.getmaxyx()
    statusbarstr = "Press 'q' to exit"
    stdscr.attron(curses.color_pair(3))
    stdscr.addstr(height-1, 0, statusbarstr)
    stdscr.addstr(height-1, len(statusbarstr), " " * (width - len(statusbarstr) - 1))
    stdscr.attroff(curses.color_pair(3))
# Rendering description
def RenderDescription(stdscr):
    focus_desc      = "Focus    : Left-Right Arrow"
    zoom_desc       = "Zoom     : Up-Down Arrow"
    motor_x_desc    = "MotorX   : 'w'-'s' Key"
    motor_y_desc    = "MotorY   : 'a'-'d' Key"
    ircut_desc      = "IRCUT    : Space"
    autofocus_desc  = "Autofocus: Enter"
    snapshot_desc   = "Snapshot : 'c' Key"

    desc_y = 1
    
    stdscr.addstr(desc_y + 1, 0, focus_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 2, 0, zoom_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 3, 0, motor_x_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 4, 0, motor_y_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 5, 0, ircut_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 6, 0, autofocus_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 7, 0, snapshot_desc, curses.color_pair(1))
# Rendering  middle text
def RenderMiddleText(stdscr,k,focuser):
    # get height and width of the window.
    height, width = stdscr.getmaxyx()
    # Declaration of strings
    title = "Arducam Controller"[:width-1]
    subtitle = ""[:width-1]
    keystr = "Last key pressed: {}".format(k)[:width-1]
    
    
    # Obtain device infomation
    focus_value = "Focus    : {}".format(focuser.get(Focuser.OPT_FOCUS))[:width-1]
    zoom_value  = "Zoom     : {}".format(focuser.get(Focuser.OPT_ZOOM))[:width-1]
    motor_x_val = "MotorX   : {}".format(focuser.get(Focuser.OPT_MOTOR_X))[:width-1]
    motor_y_val = "MotorY   : {}".format(focuser.get(Focuser.OPT_MOTOR_Y))[:width-1]
    ircut_val   = "IRCUT    : {}".format(focuser.get(Focuser.OPT_IRCUT))[:width-1]
    
    if k == 0:
        keystr = "No key press detected..."[:width-1]

    # Centering calculations
    start_x_title = int((width // 2) - (len(title) // 2) - len(title) % 2)
    start_x_subtitle = int((width // 2) - (len(subtitle) // 2) - len(subtitle) % 2)
    start_x_keystr = int((width // 2) - (len(keystr) // 2) - len(keystr) % 2)
    start_x_device_info = int((width // 2) - (len("Focus    : 00000") // 2) - len("Focus    : 00000") % 2)
    start_y = int((height // 2) - 6)
    
    # Turning on attributes for title
    stdscr.attron(curses.color_pair(2))
    stdscr.attron(curses.A_BOLD)

    # Rendering title
    stdscr.addstr(start_y, start_x_title, title)

    # Turning off attributes for title
    stdscr.attroff(curses.color_pair(2))
    stdscr.attroff(curses.A_BOLD)

    # Print rest of text
    stdscr.addstr(start_y + 1, start_x_subtitle, subtitle)
    stdscr.addstr(start_y + 3, (width // 2) - 2, '-' * 4)
    stdscr.addstr(start_y + 5, start_x_keystr, keystr)
    # Print device info
    stdscr.addstr(start_y + 6, start_x_device_info, focus_value)
    stdscr.addstr(start_y + 7, start_x_device_info, zoom_value)
    stdscr.addstr(start_y + 8, start_x_device_info, motor_x_val)
    stdscr.addstr(start_y + 9, start_x_device_info, motor_y_val)
    stdscr.addstr(start_y + 10, start_x_device_info, ircut_val)
# parse input key
def parseKey(k,focuser,auto_focus,camera):
    global image_count
    motor_step  = 5
    focus_step  = 100
    zoom_step   = 100
    if k == ord('s'):
        focuser.set(Focuser.OPT_MOTOR_Y,focuser.get(Focuser.OPT_MOTOR_Y) + motor_step)
    elif k == ord('w'):
        focuser.set(Focuser.OPT_MOTOR_Y,focuser.get(Focuser.OPT_MOTOR_Y) - motor_step)
    elif k == ord('d'):
        focuser.set(Focuser.OPT_MOTOR_X,focuser.get(Focuser.OPT_MOTOR_X) - motor_step)
    elif k == ord('a'):
        focuser.set(Focuser.OPT_MOTOR_X,focuser.get(Focuser.OPT_MOTOR_X) + motor_step)
    elif k == ord('r'):
        focuser.reset(Focuser.OPT_FOCUS)
        focuser.reset(Focuser.OPT_ZOOM)
    elif k == curses.KEY_DOWN:
        focuser.set(Focuser.OPT_ZOOM,focuser.get(Focuser.OPT_ZOOM) - zoom_step)
    elif k == curses.KEY_UP:
        focuser.set(Focuser.OPT_ZOOM,focuser.get(Focuser.OPT_ZOOM) + zoom_step)
    elif k == curses.KEY_RIGHT:
        focuser.set(Focuser.OPT_FOCUS,focuser.get(Focuser.OPT_FOCUS) + focus_step)
    elif k == curses.KEY_LEFT:
        focuser.set(Focuser.OPT_FOCUS,focuser.get(Focuser.OPT_FOCUS) - focus_step)
    elif k == 10:
        auto_focus.startFocus()
        # auto_focus.startFocus2()
        # auto_focus.auxiliaryFocusing()
        pass
    elif k == 32:
        focuser.set(Focuser.OPT_IRCUT,focuser.get(Focuser.OPT_IRCUT)^0x0001)
        pass
    elif k == ord('c'):
        #save image to file.
        cv2.imwrite("image{}.jpg".format(image_count), camera.getFrame())
        image_count += 1


# Python curses example Written by Clay McLeod
# https://gist.github.com/claymcleod/b670285f334acd56ad1c
def draw_menu(stdscr,camera):
    focuser = Focuser(1)
    auto_focus = AutoFocus(focuser,camera)
    

    k = 0
    cursor_x = 0
    cursor_y = 0

    # Clear and refresh the screen for a blank canvas
    stdscr.clear()
    stdscr.refresh()

    # Start colors in curses
    curses.start_color()
    curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_BLACK, curses.COLOR_WHITE)

    # Loop where k is the last character pressed
    while (k != ord('q')):
        # Initialization
        stdscr.clear()
        # Flush all input buffers. 
        curses.flushinp()
        # get height and width of the window.
        height, width = stdscr.getmaxyx()

        # parser input key
        parseKey(k,focuser,auto_focus,camera)

        # Rendering some text
        whstr = "Width: {}, Height: {}".format(width, height)
        stdscr.addstr(0, 0, whstr, curses.color_pair(1))

        # render key description
        RenderDescription(stdscr)
        # render status bar
        RenderStatusBar(stdscr)
        # render middle text
        RenderMiddleText(stdscr,k,focuser)
        # Refresh the screen
        stdscr.refresh()

        # Wait for next input
        k = stdscr.getch()

def main():
    #open camera
    camera = Camera()
    #open camera preview
    camera.start_preview()

    curses.wrapper(draw_menu,camera)

    camera.stop_preview()
    camera.close()

    

if __name__ == "__main__":
    main()