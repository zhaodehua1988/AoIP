#===============================================================================
# export variables
#===============================================================================
ifeq ($(CFG_HI_EXPORT_FLAG),)
    ifneq ($(KERNELRELEASE),)
    KERNEL_DIR := $(srctree)

    SDK_DIR := $(KERNEL_DIR)/../../..
    else
    SDK_DIR := $(CURDIR)/../../../..
    endif

include $(SDK_DIR)/base.mak
endif

#===============================================================================
# local variables
#===============================================================================
EXTRA_CFLAGS += $(CFG_HI_KMOD_CFLAGS)

EXTRA_CFLAGS += -I$(COMMON_UNF_INCLUDE)                 \
                -I$(COMMON_API_INCLUDE)                 \
                -I$(COMMON_DRV_INCLUDE)                 \
                -I$(MSP_UNF_INCLUDE)                    \
                -I$(MSP_API_INCLUDE)                    \
                -I$(MSP_DRV_INCLUDE)                    \
                -I$(MSP_DIR)/api/spi

KBUILD_EXTRA_SYMBOLS += $(COMMON_DIR)/drv/Module.symvers

MOD_NAME := hi_spi

obj-$(CFG_MSP_BUILDTYPE) += $(MOD_NAME).o

$(MOD_NAME)-y :=    drv_spi.o       \
                    drv_spi_intf.o

REMOVED_FILES := "*.o" "*.ko" "*.order" "*.symvers" "*.mod" "*.mod.*" "*.cmd" ".tmp_versions" "modules.builtin"

#===============================================================================
#   rules
#===============================================================================
.PHONY: all clean install uninstall

all:
	$(AT)make -C $(LINUX_DIR) ARCH=arm CROSS_COMPILE=$(CFG_HI_TOOLCHAINS_NAME)- M=$(CURDIR) modules

clean:
	$(AT)find ./ -name "*.d" $(foreach file, $(REMOVED_FILES), -o -name $(file)) | xargs rm -rf

install: all
ifneq ($(CFG_MSP_BUILDTYPE),y)
	$(AT)cp -f $(CURDIR)/$(MOD_NAME).ko $(MODULE_DIR)/
endif

uninstall:
	$(AT)rm -rf $(MODULE_DIR)/$(MOD_NAME).ko
