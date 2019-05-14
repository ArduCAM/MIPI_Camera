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
