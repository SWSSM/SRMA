CC := ~/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc

obj-m	:= rma_cpuinfo.o

KDIR	:= ~/panda/4AJ.2.2/kernel/android-3.0	
PWD	:=	$(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm	-rf *.ko
	rm	-rf *.mod.*
	rm	-rf .*.cmd
	rm	-rf *.o
	rm	-rf .tmp_versions
