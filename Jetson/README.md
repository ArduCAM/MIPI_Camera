# Native Jetson Cameras
Jetson Nano and Xavier NX natively support IMX219 with the well-tuned ISP, and then followed by IMX477. So it is suggested to use the native camera driver for best performance and image quality.  For IMX219, the camera driver and device tree are ready after a fresh installation of the official image.  But for recently released IMX477 camera, you have to install the driver package manually to replace the IMX219 driver.

The native IMX219 has a fixed focus lens, either glued or should be manually adjusted. Arducam designed a motorized focus IMX219 camera module which lens can be programmable controlled through software.
In the IMX219 folder, it contains the software script to control the Arducam motorized focus IMX219 camera module for manual focus or autofocus.

The IMX477 folder contains the Jetson Nano/Xavier NX driver package for different kernel versions.

# Jetvariety Cameras
Jetvariety is an Nvidia Jetson platform V4L2 kernel camera driver framework that can support any MIPI cameras Arducam provides which are not natively supported by the Jetson. A single-camera driver for all is the main goal of Jetvariety project, the users don't need to develop their own camera driver for Nvidia Jetson boards and even more, users can switch between different Arducam cameras without switching camera driver. Software compatibility for Jetvariety V4L2 driver is also another consideration for this project. Arducam_OBISP_MIPI_Camera_Module uses this driver on Jetson.

# Multi-Camera Solution
Using Arducam multi-camera adapter board, it allows users to connect 4 cameras for each CSI camera port. For Jetson Nano or Xavier NX develop kit, there are two CSI camera ports, so maximum 8 cameras can be connected with two Arducam multi-camera adapter board connected. 
All of these cameras connected to a single CSI port, only one camera is allowed to be activated at a time. Users should switch between cameras through GPIO control.

# PTZ Camera
PTZ means Pan/Tilt/Optical Zoom, it is an add-on feature for native Jetson cameras like IMX219 using Arducam PTZ camera adapter board. Users can use Arducam opensource software script to control the camera's Pan/Tilt and optical zoom/focus.
