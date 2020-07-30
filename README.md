# Introduction
The MIPI cameras are widely used nowadays for smartphones and many opensource platforms like Raspberry pi and Nvidia Jetson series boards.
And in order to add more mipi cameras (or other video streaming device with MIPI interface) support for these maker hardware platforms while keeping the mipi camera complex interface and protocol hidden from the user, 
Arducam team developed several camera drivers and demo codes based on different driver frameworks and their implementations.

# Driver Framework
* ## [MIPI Camera Driver for Raspberry Pi](https://github.com/ArduCAM/MIPI_Camera/tree/master/RPI)
MIPI camera driver is a close source userland camera driver with no kernel version dependency. It can connect any MIPI camera modules from Arducam. Since this driver only supports the RAW sensor, it can receive the RAW images without hardware ISP processing, users have to do software ISP in their own implementation.

* ## [Jetvariety for Jetson Nano/Xavier NX](https://github.com/ArduCAM/MIPI_Camera/tree/master/Jetson/Jetvariety)
Jetvariety is a Nvidia Jetson platform V4L2 kernel camera driver framework which can support any MIPI cameras Arducam provides.
A single-camera driver for all is the main goal of Jetvariety project, the user doesn't need to develop their own camera driver for Nvidia Jetson boards and even more, user can switch between different Arducam cameras without switching camera driver. Software compatibility for Jetvariety V4L2 driver is also another consideration for this project. [Arducam_OBISP_MIPI_Camera_Module](https://github.com/ArduCAM/Arducam_OBISP_MIPI_Camera_Module) uses this driver on Jetson.

* ## [Pivariety for Raspberry Pi](https://github.com/ArduCAM/Arducam_OBISP_MIPI_Camera_Module/tree/master/Release)
Similar to the Jetvariety, Pivariety uses the same idea to provide a single camera driver for all on the Raspberry pi platforms. It is also a V4L2 kernel driver for the software compatibility to most of the popular media player software or OpenCV. [Arducam_OBISP_MIPI_Camera_Module](https://github.com/ArduCAM/Arducam_OBISP_MIPI_Camera_Module) also uses this driver on Raspberry Pi. 

* ## Libcamera-arducam for Raspberry Pi
Coming soon.
