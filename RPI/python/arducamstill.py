import sys
import cv2 as cv
import numpy as np
import os
import arducam_mipicamera as arducam
def align_down(size, align):
    return (size & ~((align)-1))

def align_up(size, align):
    return align_down(size + align - 1, align)
def remove_padding(data, width, height, bit_width):
    buff = np.frombuffer(data, np.uint8)
    real_width = int(width / 8 * bit_width)
    align_width = align_up(real_width, 32)
    align_height = align_up(height, 16)
    buff = buff.reshape(align_height, align_width)
    buff = buff[:height, :real_width]
    buff = buff.reshape(height, real_width)
    buff = buff.astype(np.uint16) << 2
    # now convert to real 10 bit camera signal
    for byte in range(4):
        buff[:, byte::5] |= ((buff[:, 4::5] >> ((4 - byte) * 2)) & 0b11)
    # delete the unused pix
    buff = np.delete(buff, np.s_[4::5], 1)
    return buff
def choose_lens_table(i):
        switcher={
                0:'./lens_table/imx230/5344x4012.npy',
                1:'./lens_table/imx230/2672x2004.npy',
                2:'./lens_table/imx230/1920x1080.npy',
                3:'./lens_table/imx230/1336x1000.npy',
                4:'./lens_table/imx230/1280x960.npy',
                5:'./lens_table/imx230/1280x720.npy',
             }
        return switcher.get(i,"Invalid lens_table path ")

if __name__ == '__main__':
    camera = arducam.mipi_camera()
    print("Open camera...")
    mode =1
    camera.init_camera()
    camera.set_mode(mode) # chose a camera mode which yields raw10 pixel format, see output of list_format utility
    fmt = camera.get_format()
    width = fmt.get("width")
    height = fmt.get("height")
   # print(choose_lens_table(mode))
    print("Current resolution is {w}x{h}".format(w=width, h=height))
    mask  =  np.load(choose_lens_table(mode))
    rmask =  mask[:, :,  0]
    g1mask = mask[:, :, 1]
    g2mask = mask[:, :, 2]
    bmask =  mask[:,  :, 3]
    rmask =  cv.resize(rmask.astype(np.uint8), (width//2, height//2), interpolation=cv.INTER_LINEAR).astype(np.uint8)
    g1mask = cv.resize(g1mask.astype(np.uint8),(width//2, height//2), interpolation=cv.INTER_LINEAR).astype(np.uint8)
    g2mask = cv.resize(g2mask.astype(np.uint8), (width//2, height//2), interpolation=cv.INTER_LINEAR).astype(np.uint8)
    bmask =  cv.resize(bmask.astype(np.uint8), (width//2, height//2), interpolation=cv.INTER_LINEAR).astype(np.uint8)
    rmask = (rmask[:, :] >> 5) + (rmask[:, :] & 0x1F) / 32
    g1mask = (g1mask[:, :] >> 5) + (g1mask[:, :] & 0x1F) / 32
    g2mask = (g2mask[:, :] >> 5) + (g2mask[:, :] & 0x1F) / 32
    bmask = (bmask[:, :] >> 5) + (bmask[:, :] & 0x1F) / 32
    while cv.waitKey(10) != 27:
        frame = camera.capture(encoding = 'raw')
        #stream = open("./2672x2004.raw", 'rb') #test
        #image = stream.read()
        image = remove_padding(frame.data,width,height,10 )
        image[0::2, 0::2] = image[0::2, 0::2] * rmask 
        image[0::2, 1::2] = image[0::2, 1::2] * g1mask 
        image[1::2, 0::2] = image[1::2, 0::2] * g2mask 
        image[1::2, 1::2] = image[1::2, 1::2] * bmask 
        image = np.clip(image, 0, 1023)
        image = cv.cvtColor(image, 46)
        image = image >>2
        image = image.astype(np.uint8)
        image = cv.resize(image, (640, 480))
        cv.imshow("preview image", image)
        # Release memory
    del frame
    print("Close camera...")
    camera.close_camera()
