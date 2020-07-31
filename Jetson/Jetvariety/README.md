# Notice
Jetvariety is an Nvidia Jetson platform V4L2 kernel camera driver framework that can support any MIPI cameras Arducam provides which are not natively supported by the Jetson. 
If you have an IMX219 or IMX477 based camera module, please do not use this driver.

# Limitation 
Jetvariety mainly supports the RAW sensor, and only RAW image data available without ISP. Users have to do software ISP in their own implementation. For ISP support, please check [Arducam OBISP cameras](https://github.com/ArduCAM/Arducam_OBISP_MIPI_Camera_Module). 

# Hardware Requirement 
Jetvariety driver requires an additional Jetvariety adapter board to work with, which is always bundled to the camera breakout board.  If you only have our camera breakout board and don't have the Jetvariety adapter board, you have to contact us about how to get it.
