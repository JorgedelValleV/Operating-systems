#!/bin/bash
# Create a kernel module and load it into the operating system      

MODULE_NAME="chardev_leds"
MODULE_PATH="/dev/leds"

sudo rmmod $MODULE_NAME     
sudo rm $MODULE_PATH        
make clean                  
make                        

sudo insmod $MODULE_NAME.ko 
lsmod | head                

sudo mknod -m 666 $MODULE_PATH c 240 0 

sudo echo 1 > $MODULE_PATH
sleep 2
sudo echo 21 > $MODULE_PATH
sleep 2
sudo echo 2 > $MODULE_PATH

dmesg | tail