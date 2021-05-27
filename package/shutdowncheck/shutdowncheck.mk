################################################################################
#
# shutdowncheck
#
################################################################################

SHUTDOWNCHECK_VERSION = 1.0
SHUTDOWNCHECK_SITE = $(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/shutdowncheck/src
SHUTDOWNCHECK_SITE_METHOD = local

define SHUTDOWNCHECK_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define SHUTDOWNCHECK_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/shutdowncheck $(TARGET_DIR)/usr/bin
endef

define SHUTDOWNCHECK_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/shutdowncheck/init/shutdowncheck.service \
		$(TARGET_DIR)/usr/lib/systemd/system/shutdowncheck.service
endef

define SHUTDOWNCHECK_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/shutdowncheck/init/S99shutdowncheck \
		$(TARGET_DIR)/etc/init.d/S99shutdowncheck
endef

$(eval $(generic-package))