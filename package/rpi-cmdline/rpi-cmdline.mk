################################################################################
#
# rpi-cmdline
#
################################################################################

RPI_CMDLINE_DEPENDENCIES = rpi-firmware
RPI_CMDLINE_INSTALL_IMAGES = YES

define RPI_CMDLINE_INSTALL_IMAGES_CMDS
	echo $(BR2_PACKAGE_RPI_CMDLINE_TXT) > \
		$(BINARIES_DIR)/rpi-firmware/cmdline.txt
endef

$(eval $(generic-package))