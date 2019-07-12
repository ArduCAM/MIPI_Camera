# Introudction
This the MIPI camera software SDK for Raspberry pi platform, which allow you connect global shutter cameras and upto 18MP cameras on RPI board.

Now the supported MIPI camera modules are below:

* 0.3MP OV7251 Monochrome Global Shutter
 
* 1MP OV9281 Monochrome Global Shutter
 
* 2MP OV2311 Monochrome Global Shutter

* 13MP IMX135 Color Rolling Shutter

* 16MP IMX298 Color Rolling Shutter

* 18MP AR1820 Color Rolling Shutter

# Video Demo
[![IMAGE ALT TEXT](https://github.com/arducam/MIPI_Camera/blob/master/RPI/images/MIPI_Camera_RPI_Demo.jpg)](https://youtu.be/XJ2VrwXMhy4 "Up to 18MP MIPI Cameras for Raspberry Pi")       

# Prerequisites
## Enable i2c_vc
```bash
$ chmod +x ./enable_i2c_vc.sh
$ ./enable_i2c_vc.sh
```
Alter running the script, reboot will be required.

## Install the SDK library
```bash
$ make install
```
## Compile the Examples
```bash
$ make clean && make
```

## Optional Settings
Edit /boot/config.txt file.
Find gpu_mem=xxx line
Modify gpu_mem size with proper size, recommend to use 

`gpu_mem=160` for 13MP  camera board,

`gpu_mem=180` for 16MP or higher camera board.

# Running the Examples
## Preview Example
```bash
$ ./preview
```
In the preview.c example, it will demo how to do preview in different resolution and camera control parameters.

## Capture Example
```bash
$ ./capture
```
In the capture.c example, it will capture diffferent resolution JPEG images.

```bash
$ ./capture_raw
```
In the capture_raw.c example, it will capture diffferent resolution none interpolation raw format images, especially useful for monochrome sensors.

```bash
$ ./raw_callback
```
In the raw_callback.c example, it is callback version of capture_raw example.

## Video Recording Example
```bash
$ ./video
```
In the video.c example, it will record the video in H246 format.

## Camera Control Query Example
```bash
$ ./list_format
```
In the list_format.c example, it will list camera supported resolution and control functions.

## Sensor Register Read/Write Example
```bash
$ ./read_write_sensor_reg
```
In the read_write_sensor_reg.c example, it illustrates how to directly read/write sensor registers.
This example might need to be modifed according to the correct sensor register address.

## OpenCV Example
```bash
$ ./capture2opencv
```
In the capture2opencv.cpp example, it converts YUV data to OpenCV Mat format, and displays as is.

## Gstreamer Example
In the video2stdout.c example, it outputs H.264 video stream to stdout, and uses gstreamer to push the stream to PC.

### Example 1:

Raspberry pi side command:
```bash
$ ./video2stdout | nc -l -p 5000
```
PC side command: (x.x.x.x is your Raspberry Pi IP address)
```bash
$  gst-launch-1.0  -v tcpclientsrc  host=x.x.x.x  port=5000 ! decodebin ! autovideosink
```

### Example 2:

Raspberry pi side command:  (x.x.x.x is your Raspberry Pi IP address)
```bash
$ ./video2stdout | gst-launch-1.0 -v fdsrc ! h264parse !  rtph264pay config-interval=1 pt=96 ! gdppay ! tcpserversink host=x.x.x.x port=5000
```
PC side command: (x.x.x.x is your Raspberry Pi IP address)
```bash
$  gst-launch-1.0 -v tcpclientsrc host=x.x.x.x port=5000 ! gdpdepay ! rtph264depay ! avdec_h264 ! autovideosink sync=false
```

## QR Code Detection Example
```bash
$ ./qrcode_detection <exposure_value>
```
In the qrcode_detection.cpp example, it illustrates how to use global shutter camera like OV7251 or OV9281 to detect QR code using OpenCV.
To run this demo you have to install the dependence 

`sudo apt-get update && sudo apt-get install libzbar-dev libopencv-dev`

## Dual Camera Demo
```bash
$ ./preview-dualcam
or
$ ./capture-dualcam
```
In the preview-dualcam.c examle, it illustrates how to open the two camera ports on Raspberry pi compute module at the same time for preview.
And the capture-dualcam.c examle, it illustrates how to do capture from each camera port on Raspberry pi compute module by switching between them.

A camera_interface struct should be constructed according to your hardware wiring.

For example camera port 0 is using sda_pin 28, scl_pin 29, led_pin 30, shutdown_pin 31, and camera port 1 is using sda_pin 0, scl_pin 1, led_pin 2, shutdown_pin 3.

More information about the compute module wiring please check : https://www.raspberrypi.org/documentation/hardware/computemodule/cmio-camera.md 

# Python Wrapper and Examples
The arducam_mipicamera.py script is a wrapper for the libarducam_mipicamera.so dynamic library. 
To use this script you need to pre-install libarducam_mipicamera.so.
All python examples are in the Python Folder.

## Dependency
`sudo pip install v4l2`

`sudo pip install numpy`

`sudo apt-get install python-opencv`

# Utility
## How to playback the H264 file
1. Compile hello_video.bin
```bash
$ cd /opt/vc/src/hello_pi && ./rebuild.sh
```

2. Play H264 file
```bash
$ /opt/vc/src/hello_pi/hello_video/hello_video.bin test.h264
```

## How to view RAW data
In the utils folder, there are two python script to read and display RAW image.

`mipi_raw10_to_jpg.py` is used to display color RAW image.

`mono_to_jpg.py` is used to display monochrome RAW iamge.

        