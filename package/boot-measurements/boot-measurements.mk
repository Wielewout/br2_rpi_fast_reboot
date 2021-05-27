################################################################################
#
# boot-measurements
#
################################################################################

BOOT_MEASUREMENTS_VERSION = 1.0
BOOT_MEASUREMENTS_SITE = $(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/boot-measurements/src
BOOT_MEASUREMENTS_SITE_METHOD = local
BOOT_MEASUREMENTS_DEPENDENCIES = rpi-firmware
BOOT_MEASUREMENTS_INSTALL_IMAGES = YES

define BOOT_MEASUREMENTS_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define BOOT_MEASUREMENTS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/print-measurements $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m 0755 $(@D)/clear-measurements $(TARGET_DIR)/usr/bin
endef

define BOOT_MEASUREMENTS_INSTALL_IMAGES_CMDS
	# remove serial boot info
	sed -i -e 's/console=ttyAMA0,115200//g' \
		$(BINARIES_DIR)/rpi-firmware/cmdline.txt
endef

$(eval $(generic-package))