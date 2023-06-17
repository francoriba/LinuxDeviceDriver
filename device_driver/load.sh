make
sudo insmod gpio_drv.ko
sudo lsmod | grep gpio_drv
sudo chmod 666 /dev/gpio_device
