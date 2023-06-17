obj-m +=  gpio_drv.o

# Kernel Sources
KDIR := /lib/modules/$(shell uname -r)/build

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

# Remember to change device file permissions after insterting the module
# "sudo chmod 666 /dev/gpio_device"