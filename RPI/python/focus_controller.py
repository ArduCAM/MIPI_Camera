import numpy as py
import os
import sys
import time
import curses
import arducam_mipicamera as arducam
import v4l2 #sudo pip install v4l2

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
    focus_desc      = "Focus    : Up-Down Arrow"
    snapshot_desc   = "Snapshot : 'c' Key"
    reset_desc       = "Reset    : 'r' Key"

    desc_y = 1
    
    stdscr.addstr(desc_y + 1, 0, focus_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 2, 0, snapshot_desc, curses.color_pair(1))
    stdscr.addstr(desc_y + 3, 0, reset_desc, curses.color_pair(1))
# Rendering  middle text
def RenderMiddleText(stdscr,k, camera):
    # get height and width of the window.
    height, width = stdscr.getmaxyx()
    # Declaration of strings
    title = "Arducam Controller"[:width-1]
    subtitle = ""[:width-1]
    keystr = "Last key pressed: {}".format(k)[:width-1]
    
    
    # Obtain device infomation
    try:
        fv = camera.get_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
    except:
        fv = 10
    
    focus_value = "Focus    : {}".format(fv)[:width-1]
    
    if k == 0:
        keystr = "No key press detected..."[:width-1]

    # Centering calculations
    start_x_title = int((width // 2) - (len(title) // 2) - len(title) % 2)
    start_x_subtitle = int((width // 2) - (len(subtitle) // 2) - len(subtitle) % 2)
    start_x_keystr = int((width // 2) - (len(keystr) // 2) - len(keystr) % 2)
    start_x_device_info = int((width // 2) - (len("Focus    : 00000") // 2) - len("Focus    : 00000") % 2)
    start_y = int((height // 2) - 4)
    
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

# parse input key
def parseKey(k, camera):
    global image_count
    focus_step  = 10
    try:
        if k == ord('r'):
            camera.reset_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
        elif k == curses.KEY_DOWN:
            focus_value = camera.get_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
            if focus_value - focus_step < 0:
                camera.set_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE, 0)
            else:
                camera.set_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE, focus_value - focus_step)

        elif k == curses.KEY_UP:
            focus_value = camera.get_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
            camera.set_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE, focus_value + focus_step)

        elif k == ord('c'):
            frame = camera.capture(encoding = 'jpeg')
            frame.as_array.tofile("image{}.jpg".format(image_count))
            #save image to file.
            image_count += 1
    except Exception as e:
        pass



# Python curses example Written by Clay McLeod
# https://gist.github.com/claymcleod/b670285f334acd56ad1c
def draw_menu(stdscr,camera):

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
        parseKey(k, camera)

        # Rendering some text
        whstr = "Width: {}, Height: {}".format(width, height)
        stdscr.addstr(0, 0, whstr, curses.color_pair(1))

        # render key description
        RenderDescription(stdscr)
        # render status bar
        RenderStatusBar(stdscr)
        # render middle text
        RenderMiddleText(stdscr,k,camera)
        # Refresh the screen
        stdscr.refresh()

        # Wait for next input
        k = stdscr.getch()

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
        print("The camera may not support this control.")

def main():
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
        curses.wrapper(draw_menu,camera)
        print("Stop preview...")
        camera.stop_preview()
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)

if __name__ == "__main__":
    main()