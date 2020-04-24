#coding=utf-8
#-*- coding:utf-8 -*-

'''
        Step1: Read the original image data from ./originalImage/1160x800.raw"
        Step2: Create lens shading table using gaussian_pyramid function
        Step3: Open the camera   
        Step4: process lens shading using the tabel we create
        Step5: fix awb
        step6: show the image processed. 
'''
import arducam_mipicamera as arducam
import cv2 as cv
import numpy as np 
import time
def set_controls(camera):
    try:
        print("Reset the focus...")
        camera.reset_control(0x009A090A)
        camera.set_control(0x00980911, 3000)
        camera.set_control(0x009A090A,150)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")

    try:
        print("Enable Auto Exposure...")
        camera.software_auto_exposure(enable = False)
        print("Enable Auto White Balance...")
        #camera.software_auto_white_balance(enable = True)
    except Exception as e:
        print(e)
def gaussian_pyramid(img, depth):
    G = img.copy()
    gp = [G]
    for i in range(1,depth):
        G = cv.pyrDown(G)
        gp.append(G)
    return gp
if __name__ == '__main__':
    try:
        '''
        Get the original image data and delete the garbage
        '''
        # Open and get the raw image file
        print("This script is used for fix lens shading.")
        stream = open ("./originalImage/1160x800.raw",'rb') 
        rawImageData = stream.read() 
        rawImageData = np.frombuffer(rawImageData,dtype=np.uint8)

        #delete the unused data reshape((height, width))
        rawImageData = rawImageData.reshape((800, 1472))[:800, :1450]
        '''
        Unpack raw10 data
        Raw10 data format:
        Byte0  | Byte1  | Byte2  | Byte3  |     Byte4                   | Byte5   |....   
        P1[9:2]| P2[9:2]| P3[9:2]| P4[9:2]| P1[1:0]P2[1:0]P3[1:0]P4[1:0]| P5[9:2] |....
        '''
        rawImageData = rawImageData.astype(np.uint16)<< 2
        # now convert to real 10 bit camera signal
        for byte in range(4):
            rawImageData[:, byte::5] |= ((rawImageData[:, 4::5] >> ((4 - byte) * 2)) & 0b11)
        #delete the unused pix
        rawImageData = np.delete(rawImageData,np.s_[4::5], 1)
        cplane = np.zeros((rawImageData.shape[0] // 2, rawImageData.shape[1] // 2, 4), dtype=rawImageData.dtype) # 申请4个buf 用来存放 各个通道的增益值

        cplane[:,:,0] = rawImageData[1::2, 1::2]  #R gain
        cplane[:,:,1] = rawImageData[0::2, 1::2]  #G gain
        cplane[:,:,2] = rawImageData[1::2, 0::2]  #G gain
        cplane[:,:,3] = rawImageData[0::2, 0::2]  #B gain
        scaler = 64
        res = gaussian_pyramid(cplane,6)
        raw = res[5]
        center = np.amax(np.amax(raw, axis=0),axis=0) 
        mask = scaler*center/raw
        mask = mask.astype(int).clip(0,0xFF)
        rmask =  mask[:,:,0]
        g1mask = mask[:,:,1]
        g2mask = mask[:,:,2]
        bmask=  mask[:,:,3]
        print(rmask.shape)
        rmask = cv.resize(rmask.astype(np.uint8),(580,400),interpolation = cv.INTER_LINEAR).astype(np.uint8)
        g1mask = cv.resize(g1mask.astype(np.uint8),(580,400),interpolation = cv.INTER_LINEAR).astype(np.uint8)
        g2mask = cv.resize(g2mask.astype(np.uint8),(580,400),interpolation = cv.INTER_LINEAR).astype(np.uint8)
        bmask = cv.resize(bmask.astype(np.uint8),(580,400),interpolation = cv.INTER_LINEAR).astype(np.uint8)
        rmask = (rmask[:, :] >> 5) + (rmask[:, :] & 0x1F) / 32
        g1mask = (g1mask[:, :] >> 5) + (g1mask[:, :] & 0x1F) / 32
        g2mask = (g2mask[:, :] >> 5) + (g2mask[:, :] & 0x1F) / 32
        bmask = (bmask[:, :] >> 5) + (bmask[:, :] & 0x1F) / 32
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        fmt = camera.set_resolution(1160,800)
        set_controls(camera)
        height = 800
        width = 1472
        while cv.waitKey(10) != 27:
            start = time.time()
            frame = camera.capture(encoding = 'raw')
           # print("width: %d" %width)
            image = frame.as_array.reshape(height, width)[:800,:1450]
            '''
            Byte0  | Byte1  | Byte2  | Byte3  |     Byte4                   | Byte5   |....   
            P1[9:2]| P2[9:2]| P3[9:2]| P4[9:2]| P1[1:0]P2[1:0]P3[1:0]P4[1:0]| P5[9:2] |....
            '''
            image = image.astype(np.uint16)<< 2
            # now convert to real 10 bit camera signal
            for byte in range(4):
                image[:, byte::5] |= ((image[:, 4::5] >> ((4 - byte) * 2)) & 0b11)
            #delete the unused pix
            rawImageData = np.delete(image,np.s_[4::5], 1)
            '''
            Fix lensShasing 
            '''
            rawImageData[1::2, 1::2] = rawImageData[1::2, 1::2]*rmask/2
            rawImageData[0::2, 1::2] = rawImageData[0::2, 1::2]*g1mask/2
            rawImageData[1::2, 0::2] = rawImageData[1::2, 0::2]*g2mask/2
            rawImageData[0::2, 0::2] = rawImageData[0::2, 0::2]*bmask/2
            rawImageData = np.clip(rawImageData,0,1023)
          
            '''
            Fix AWB
            '''
            awbImg = cv.cvtColor(rawImageData, 46)>>2
            awbImg = awbImg.astype(np.uint8)
            r, g, b = cv.split(awbImg)
            r_avg = cv.mean(r)[0]
            g_avg = cv.mean(g)[0]
            b_avg = cv.mean(b)[0]
    
            k = (r_avg + g_avg + b_avg) / 3
            kr = k / r_avg
            kg = k / g_avg
            kb = k / b_avg
            r = cv.addWeighted(src1=r, alpha=kr, src2=0, beta=0, gamma=0)
            r= np.clip(r,0,255)
            g = cv.addWeighted(src1=g, alpha=kg, src2=0, beta=0, gamma=0)
            g= np.clip(g,0,255)
            b = cv.addWeighted(src1=b, alpha=kb, src2=0, beta=0, gamma=0)
            b= np.clip(g,0,255)
            balance_img = cv.merge([b, g, r])
            balance_img = cv.resize(balance_img,(640,480))
            '''
            Show the image processed.
            '''
            cv.imshow("awb stream",balance_img)
            print(time.time() - start)
        # Release memory
        del frame
        print("Close camera...")
        camera.close_camera()
    except Exception as e:
        print(e)