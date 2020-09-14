KPROJ=cachegrab

# Set this variable to the location of the Linux kernel
KERNEL_DIR=/lib/modules/4.6.3

ifndef KERNEL_DIR
$(error Set KERNEL_DIR variable in Makefile)
endif

obj-m += cachegrab.o
cachegrab-objs := src/cachegrab.o
cachegrab-objs += src/cachegrab_ioctl.o
cachegrab-objs += src/scope.o src/probes.o
cachegrab-objs += src/probe_generic.o
cachegrab-objs += src/my_cache.o
cachegrab-objs += src/probe_l1d.o src/probe_l1i.o src/probe_btb.o
cachegrab-objs += src/memory.o

$(KPROJ):
	make ARCH=arm64 CFLAGS_MODULE=-fno-pic -C $(KERNEL_DIR)/build M=$(PWD) modules

deploy: $(KPROJ)
	adb shell su -c "rmmod $(KPROJ)"
	adb push $(KPROJ).ko /data/local/tmp/$(KPROJ).ko
	adb shell su -c "insmod /data/local/tmp/$(KPROJ).ko"

style:
	indent -linux src/*.c src/*.h

clean:
	rm -rf *.o, *.mod.* src/*.o
