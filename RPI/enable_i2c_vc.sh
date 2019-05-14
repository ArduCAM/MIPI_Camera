#!/bin/bash
# load i2c-dev moudle
sudo modprobe i2c-dev
# add dtparam=i2c_vc=on to /boot/config.txt
awk 'BEGIN{ count=0 }       \
{                           \
    if($1 == "dtparam=i2c_vc=on"){       \
        count++;            \
    }                       \
}END{                       \
    if(count <= 0){         \
        system("sudo sh -c '\''echo dtparam=i2c_vc=on >> /boot/config.txt'\''"); \
    }                       \
}' /boot/config.txt

echo "reboot now?(y/n):"
read USER_INPUT
case $USER_INPUT in
'y'|'Y')
    echo "reboot"
    sudo reboot
;;
*)
    echo "cancel"
;;
esac

        
