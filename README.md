# Introudction
The MIPI cameras are widely used nowdays for smart phones and many opensource platforms like Raspberry pi and Nvidia Jetson series boards.
And in order to add more mipi cameras (or other video streaming device with MIPI interface) support for these maker hardware paltforms while keeping the mipi camera complex interface and protocol hidden from user, 
Arducam team developed several camera drivers and demo code based on different driver frameworks and their implementations.

# Driver Framework
* ## MIPI Camera Driver for Raspberry Pi
MIPI camera driver is a close source userland camera driver with no kernel verion dependency. It can connect any MIPI camera modules from Arducam. Since this driver only supports the RAW sensor, it can receive the RAW images without hardware ISP processing, users have to do software ISP in their own implementation.

* ## Jetvariety for Jetson Nano/Xavier NX
Jetvariety is a Nvidia Jetson platorms V4L2 kernel camera driver framework which can support any MIPI cameras Arducam provides.
A single camera driver for all is the main goal of Jetvariety project, user doesn't need to develop their own camera driver for Nvidia Jetson boards and 
even more user can switch between different Arducam cameras without switching camera driver. Software compatibility for Jetvariety V4L2 driver is also another consideration for this project.

* ## Pivariety for Raspberry Pi
Similar to the Jetvariety, Pivariety uses the same idea to provide a single camera driver for all on the Raspberry pi platforms. It is also a V4L2 kernel driver for the software compatibility to most of the popular media player software or OpenCV.  

* ## Libcamera-arducam for Raspberry Pi
Coming soon.
