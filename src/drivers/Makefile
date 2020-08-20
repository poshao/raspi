#ARCH=arm
#CROSS_COMPILE=/workplace/compile/raspiTools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-
#KERN_DIR =/home/shaozi/workplace/compile/raspiLinux
# KERN_DIR = ~/linux
MAKE=make

ifneq ($(KERNELRELEASE),)
    obj-m := ssd1306.o
else
    KERN_DIR ?= /usr/src/linux-headers-$(shell uname -r)/
    PWD := $(shell pwd)
endif

default:
# -C后面是编译时进入的目录，M=是编译完成后回到的目录
	$(MAKE) -C $(KERN_DIR) M=$(PWD) modules
	-sudo rmmod ssd1306
	sudo cp ./ssd1306.ko /lib/modules/$(shell uname -r)/
	sudo depmod

	# sudo cp ./ssd1306.ko /lib/modules/$(uname -r)/ssd1306.ko
	# sudo depmod ssd1306.ko

initconfig:
# 需要加载内核默认配置，根据不同平台选择对应的默认配置
	$(MAKE) -C $(KERN_DIR) bcm2711_defconfig
# 编译下内核，不然编译模块时会出错
	$(MAKE) -C  $(KERN_DIR)

loadDriver:
	sudo cp ./ssd1306.ko /lib/modules/$(shell uname -r)/
	sudo depmod
	sudo modprobe ssd1306

unloadDriver:
	sudo rmmod ssd1306
	sudo rm /lib/modules/$(shell uname -r)/ssd1306.ko
	sudo depmod

loadDev:
	dtc -I dts -O dtb -o ./dtoverlay/ssd1306-spi.dtbo ./dtoverlay/ssd1306-spi.dts
	sudo dtoverlay ./dtoverlay/ssd1306-spi.dtbo
	dmesg

unloadDev:
	sudo dtoverlay -R ssd1306-spi

.PHONY:clean
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions