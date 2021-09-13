obj-m += gpio_write.o


KDIR = $(HOME)/yocto/fsl-release-bsp/customer/build
CROSS= $(HOME)/yocto/fsl-release-bsp/buildull/tmp/sysroots/x86_64-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-

all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR) M=$(PWD) modules

clean:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR) M=$(PWD) clean
