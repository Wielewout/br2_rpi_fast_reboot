################################################################################
#
# periodic-measurements
#
################################################################################

PERIODIC_MEASUREMENTS_VERSION = 1.0
PERIODIC_MEASUREMENTS_SITE = $(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/periodic-measurements/src
PERIODIC_MEASUREMENTS_SITE_METHOD = local
PERIODIC_MEASUREMENTS_DEPENDENCIES = rpi-localizer

define PERIODIC_MEASUREMENTS_BUSYBOX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_PKILL)
endef

define PERIODIC_MEASUREMENTS_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(patsubst "%",%,$(BR2_PACKAGE_PERIODIC_MEASUREMENTS_TYPE))-measurements
endef

define PERIODIC_MEASUREMENTS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 \
	$(@D)/$(patsubst "%",%,$(BR2_PACKAGE_PERIODIC_MEASUREMENTS_TYPE))-measurements \
	$(TARGET_DIR)/usr/bin
endef

define PERIODIC_MEASUREMENTS_INSTALL_INIT_SYSTEMD
	rm -f $(TARGET_DIR)/usr/lib/systemd/system/localizer.service
	$(INSTALL) -D -m 644 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/periodic-measurements/init/periodic-measurements.service \
		$(TARGET_DIR)/usr/lib/systemd/system/periodic-measurements.service
	sed -i \
		-e 's/%%MEASUREMENT_TYPE%%/$(patsubst "%",%,$(BR2_PACKAGE_PERIODIC_MEASUREMENTS_TYPE))/g' \
		$(TARGET_DIR)/usr/lib/systemd/system/periodic-measurements.service
endef

define PERIODIC_MEASUREMENTS_INSTALL_INIT_SYSV
	rm -f $(TARGET_DIR)/etc/init.d/S90localizer
	$(INSTALL) -D -m 755 \
		$(BR2_EXTERNAL_RPI_FAST_REBOOT_PATH)/package/periodic-measurements/init/S90periodicmeasurements \
		$(TARGET_DIR)/etc/init.d/S90periodicmeasurements
	sed -i \
		-e 's/%%MEASUREMENT_TYPE%%/$(patsubst "%",%,$(BR2_PACKAGE_PERIODIC_MEASUREMENTS_TYPE))/g' \
		$(TARGET_DIR)/etc/init.d/S90periodicmeasurements
endef

$(eval $(generic-package))