################################################################################
#
# rpi-localizer
#
################################################################################

RPI_LOCALIZER_VERSION = 1.0
RPI_LOCALIZER_SITE = $(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/rpi-localizer/src
RPI_LOCALIZER_SITE_METHOD = local
RPI_LOCALIZER_DEPENDENCIES = rpi-firmware rpi-userland

RPI_LOCALIZER_VC_LIB = $(BUILD_DIR)/rpi-firmware-$(RPI_FIRMWARE_VERSION)/opt/vc/lib
RPI_LOCALIZER_USERLAND_ROOT = $(BUILD_DIR)/rpi-userland-$(RPI_USERLAND_VERSION)

RPI_LOCALIZER_INCLUDE_PATHS += -I./inc
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_VC_LIB)
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)/interface/vcos
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)/interface/vcos/pthreads
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)/interface/vmcs_host/linux
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)/host_applications/linux/libs/sm
RPI_LOCALIZER_INCLUDE_PATHS += -I$(RPI_LOCALIZER_USERLAND_ROOT)/host_applications/linux/libs/bcm_host/include

RPI_LOCALIZER_LIB_PATHS += -L$(RPI_LOCALIZER_VC_LIB)

define RPI_LOCALIZER_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) \
		USERLAND_ROOT="$(RPI_LOCALIZER_USERLAND_ROOT)" \
		INCLUDE_PATHS="$(RPI_LOCALIZER_INCLUDE_PATHS)" \
		CFLAGS+="$(RPI_LOCALIZER_INCLUDE_PATHS)" \
		LIB_PATHS="$(RPI_LOCALIZER_LIB_PATHS)" \
		-C $(@D) \
		all
endef

define RPI_LOCALIZER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/localizer $(TARGET_DIR)/usr/bin
	$(INSTALL) -d $(TARGET_DIR)/usr/share/glsl
	$(INSTALL) -D \
		$(BUILD_DIR)/rpi-localizer-$(RPI_LOCALIZER_VERSION)/glsl/binfragshader.glsl.c \
		$(TARGET_DIR)/usr/share/glsl/binfragshader.glsl.c
endef

define RPI_LOCALIZER_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/rpi-localizer/init/localizer.service \
		$(TARGET_DIR)/usr/lib/systemd/system/localizer.service
endef

define RPI_LOCALIZER_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/rpi-localizer/init/S90localizer \
		$(TARGET_DIR)/etc/init.d/S90localizer
endef

$(eval $(generic-package))