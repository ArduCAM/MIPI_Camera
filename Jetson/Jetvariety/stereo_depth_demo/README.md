OpenCV and Depth Map on Arducam Stereo Camera HAT
===========
(base on [stereopi-tutorial](https://github.com/realizator/stereopi-tutorial))


## Was tested in the following environment:
* JetPack 4.4 L4T32.4.2 ([SD Card Image](https://developer.nvidia.com/jetson-nano-sd-card-image))
* Python 2.7.17 (SD Card Image built-in)
* Python 3.6.9 (SD Card Image built-in)
* OpenCV 4.1.1 (SD Card Image built-in)

## Installing Python2.7 Dependencies
* wget https://bootstrap.pypa.io/get-pip.py && sudo python get-pip.py
* sudo pip install stereovision
* sudo apt install libpng-dev
* sudo apt install libfreetype6-dev
* sudo apt install python-gi-cairo
* sudo pip install matplotlib

## Installing Python3.x Dependencies
* wget https://bootstrap.pypa.io/get-pip.py && sudo python3 get-pip.py
* sudo pip3 install stereovision
* sudo apt install libpython3-dev
* sudo pip3 install matplotlib 


**See [Depth Mapping on Arducam Stereo Camera HAT with OpenCV](https://www.arducam.com/docs/cameras-for-raspberry-pi/synchronized-stereo-camera-hat/opencv-and-depth-map-on-arducam-stereo-camera-hat-tutorial/) for details.**