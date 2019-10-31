## Jetson Naono Multi Camera Adapter

### Install GPIO lib
sudo pip install Jetson.GPIO

### Setting User Permissions
In order to use the Jetson GPIO Library, the correct user permissions/groups must be set first.

Create a new gpio user group. Then add your user to the newly created group.

`sudo groupadd -f -r gpio`  
`sudo usermod -a -G gpio $USER`  
Install custom udev rules by copying the 99-gpio.rules file into the rules.d directory:

`sudo cp /opt/nvidia/jetson-gpio/etc/99-gpio.rules /etc/udev/rules.d/`  

Please note that for the new rule to take place, you may either need to reboot or reload the udev rules by issuing this command:

`sudo udevadm control --reload-rules && sudo udevadm trigger`  
`sudo reboot`  

Visiting here to get more detail information[Jetson.GPIO - Linux for Tegra](https://pypi.org/project/Jetson.GPIO/#description)

### Run demo 
`sudo python JetsonNanoAdapterTestDemo.py`  
This demo will preview each camera and get one snapshot. Then stored it to local path.