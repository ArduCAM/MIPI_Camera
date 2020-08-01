# Notice
Jetvariety is an Nvidia Jetson platform V4L2 kernel camera driver framework that can support any MIPI cameras Arducam provides which are not natively supported by the Jetson. 
If you have an IMX219 or IMX477 based camera module, please do not use this driver.

# Limitation 
Jetvariety mainly supports the RAW sensor, and only RAW image data available without ISP. Users have to do software ISP in their own implementation. For ISP support, please check [Arducam OBISP cameras](https://github.com/ArduCAM/Arducam_OBISP_MIPI_Camera_Module). 

# Hardware Requirement 
Jetvariety driver requires an additional [Jetvariety adapter board](https://www.arducam.com/jetson-nano-one-arducam-driver-support-all-camera-sensor/?highlight=jetvariety) to work with, which is always bundled to the camera breakout board.  If you only have our camera breakout board and don't have the Jetvariety adapter board, you have to contact us about how to get it.

# Example Code
There are several examples of code to demonstrate how to use Jetvariety cameras.
1. In the [example](https://github.com/ArduCAM/MIPI_Camera/tree/master/Jetson/Jetvariety/example) folder, there are python scripts to open and display the camera video in realtime.

2. In the [external_trigger](https://github.com/ArduCAM/MIPI_Camera/tree/master/Jetson/Jetvariety/external_trigger) folder, there is an example of enabling the trigger mode (for OV7251/OV9281/OV2311 cameras) and display the video feed according to the RAW image data bit-depth. 

3. In the [stereo_depth_demo] (https://github.com/ArduCAM/MIPI_Camera/tree/master/Jetson/Jetvariety/stereo_depth_demo) folder, there are complete steps to create depth sensing using [stereo camera HAT](https://www.arducam.com/dual-camera-hat-synchronize-stereo-pi-raspberry).
