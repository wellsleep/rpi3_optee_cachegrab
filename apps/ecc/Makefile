PWD := $(shell pwd)

export V ?= 0

CROSS_COMPILE = 
export TA_DEV_KIT_DIR = $(PWD)/../ta_sdk
export TEEC_EXPORT = $(PWD)/../ca_sdk

# If _HOST or _TA specific compilers are not specified, then use CROSS_COMPILE
HOST_CROSS_COMPILE ?= $(CROSS_COMPILE)
TA_CROSS_COMPILE ?= $(CROSS_COMPILE)

.PHONY: all
all:
	$(MAKE) -C host CROSS_COMPILE="$(HOST_CROSS_COMPILE)"
	$(MAKE) -C ta CROSS_COMPILE="$(TA_CROSS_COMPILE)"
	cp host/optee_* $(PWD)/../out/ca/.
	cp ta/*.ta $(PWD)/../out/ta/.

.PHONY: clean
clean:
	$(MAKE) -C host clean
	$(MAKE) -C ta clean
