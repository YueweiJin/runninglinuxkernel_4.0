#!/bin/sh

if [ $# -lt 2  ]; then
	echo "Usage: $0 [arch] [option]"
fi

case $1 in
	arm32)
		export ARCH=arm
		export CROSS_COMPILE=arm-linux-gnueabi-
		if [ ! -f .config ];then
			#make vexpress_defconfig
			#make olddefconfig
			cp config_arm32_jyw .config
		fi
		make bzImage -j4 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
		make dtbs;;
	arm64)
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		make menuconfig
		make -j4
esac

