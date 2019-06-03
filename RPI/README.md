# Introudction
This the MIPI camera software SDK for Raspberry pi platform, which allow you connect global shutter cameras and upto 16MP cameras on RPI board.

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
Modify gpu_mem size with proper size, recommend to use gpu_mem=160 for 13MP or higher camera board.

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

$ ./capture_raw
```
In the capture_raw.c example, it will capture diffferent resolution none interpolation raw format images, especially useful for monochrome sensors.

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

##提示:
* 如何使用树梅派播放H264文件
    1. 编译hello_video.bin
        `cd /opt/vc/src/hello_pi && ./rebuild.sh`
    2. 播放H264文件
        `/opt/vc/src/hello_pi/hello_video/hello_video.bin test.h264`
        
# Utility
## How to playback the H264 file
1. Compile hello_video.bin
`cd /opt/vc/src/hello_pi && ./rebuild.sh`

2. Play H264 file
`/opt/vc/src/hello_pi/hello_video/hello_video.bin test.h264` 

## How to view RAW data
In the utils folder, there are two python script to read and display RAW image.

mipi_raw10_to_jpg.py is used to display color RAW image.

mono_to_jpg.py is used to display monochrome RAW iamge.       
        