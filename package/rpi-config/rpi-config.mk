################################################################################
#
# rpi-config
#
################################################################################

RPI_CONFIG_DEPENDENCIES = rpi-firmware
RPI_CONFIG_INSTALL_IMAGES = YES

# https://stackoverflow.com/questions/20267910/how-to-add-a-line-in-sed-if-not-match-is-found
# call enable-option with parameters: $1=name $2=value
define enable-option
    sed -i \
        -e '/^\#\?\(\s*'"$(1)"'\s*=\s*\).*/{s//\1'"$(2)"'/;:a;n;ba;q}' \
        -e '$$a'"$(1)"'='"$(2)" \
		$(BINARIES_DIR)/rpi-firmware/config.txt
    echo "$(1)=$(2) written in config.txt"
endef

# ----------------------------------- MEMORY -----------------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_GPU_MEM),y)
define RPI_CONFIG_GPU_MEM
	$(call enable-option,gpu_mem,$(BR2_PACKAGE_RPI_CONFIG_GPU_MEM_VALUE))
endef
endif

ifdef BR2_PACKAGE_RPI_CONFIG_GPU_MEM_256
define RPI_CONFIG_GPU_MEM_256
	$(call enable-option,gpu_mem_256,$(BR2_PACKAGE_RPI_CONFIG_GPU_MEM_256))
endef
endif

ifdef BR2_PACKAGE_RPI_CONFIG_GPU_MEM_512
define RPI_CONFIG_GPU_MEM_512
	$(call enable-option,gpu_mem_512,$(BR2_PACKAGE_RPI_CONFIG_GPU_MEM_512))
endef
endif

ifdef BR2_PACKAGE_RPI_CONFIG_GPU_MEM_1024
define RPI_CONFIG_GPU_MEM_1024
	$(call enable-option,gpu_mem_1024,$(BR2_PACKAGE_RPI_CONFIG_GPU_MEM_1024))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_TOTAL_MEM),y)
define RPI_CONFIG_TOTAL_MEM
	$(call enable-option,total_mem,$(BR2_PACKAGE_RPI_CONFIG_TOTAL_MEM_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DISABLE_L2CACHE),y)
define RPI_CONFIG_DISABLE_L2CACHE
	$(call enable-option,disable_l2cache,1)
endef
endif

# ----------------------------- LICENSE KEYS/CODEC -----------------------------

ifneq ($(BR2_PACKAGE_RPI_CONFIG_DECODE_MPG2),"")
define RPI_CONFIG_DECODE_MPG2
	$(call enable-option,decode_MPG2,$(BR2_PACKAGE_RPI_CONFIG_DECODE_MPG2))
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_DECODE_WVC1),"")
define RPI_CONFIG_DECODE_WVC1
	$(call enable-option,decode_WVC1,$(BR2_PACKAGE_RPI_CONFIG_DECODE_WVC1))
endef
endif

# -------------------------------- VIDEO/DISPLAY -------------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_HDMI_SAFE),y)
define RPI_CONFIG_HDMI_SAFE
	$(call enable-option,hdmi_safe,1)
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DISABLE_OVERSCAN),y)
define RPI_CONFIG_DISABLE_OVERSCAN
	$(call enable-option,disable_overscan,1)
endef
endif

# ----------------------------------- AUDIO ------------------------------------

# ----------------------------------- CAMERA -----------------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DISABLE_CAMERA_LED),y)
define RPI_CONFIG_DISABLE_CAMERA_LED
	$(call enable-option,disable_camera_led,1)
endef
endif

# ------------------------------------ BOOT ------------------------------------

ifdef BR2_PACKAGE_RPI_CONFIG_START_FILE
define RPI_CONFIG_START_FILE
	$(call enable-option,start_file,$(BR2_PACKAGE_RPI_CONFIG_START_FILE))
endef
endif

ifdef BR2_PACKAGE_RPI_CONFIG_FIXUP_FILE
define RPI_CONFIG_FIXUP_FILE
	$(call enable-option,fixup_file,$(BR2_PACKAGE_RPI_CONFIG_FIXUP_FILE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DISABLE_COMMANDLINE_TAGS),y)
define RPI_CONFIG_DISABLE_COMMANDLINE_TAGS
	$(call enable-option,disable_commandline_tags,1)
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_CMDLINE),"")
define RPI_CONFIG_CMDLINE
	$(call enable-option,cmdline,$(BR2_PACKAGE_RPI_CONFIG_CMDLINE))
endef
endif

ifdef BR2_PACKAGE_RPI_CONFIG_KERNEL
define RPI_CONFIG_KERNEL
	$(call enable-option,kernel,$(BR2_PACKAGE_RPI_CONFIG_KERNEL))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_ARM_64BIT),y)
define RPI_CONFIG_ARM_64BIT
	$(call enable-option,arm_64bit,1)
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_ARMSTUB),"")
define RPI_CONFIG_ARMSTUB
	$(call enable-option,armstub,$(BR2_PACKAGE_RPI_CONFIG_ARMSTUB))
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_BOOTCODE_DELAY),0)
define RPI_CONFIG_BOOTCODE_DELAY
	$(call enable-option,bootcode_delay,$(BR2_PACKAGE_RPI_CONFIG_BOOTCODE_DELAY))
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_BOOT_DELAY),1)
define RPI_CONFIG_BOOT_DELAY
	$(call enable-option,boot_delay,$(BR2_PACKAGE_RPI_CONFIG_BOOT_DELAY))
endef
endif

ifneq ($(BR2_PACKAGE_RPI_CONFIG_BOOT_DELAY_MS),0)
define RPI_CONFIG_BOOT_DELAY_MS
	$(call enable-option,boot_delay_ms,$(BR2_PACKAGE_RPI_CONFIG_BOOT_DELAY_MS))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DISABLE_SPLASH),y)
define RPI_CONFIG_DISABLE_SPLASH
	$(call enable-option,disable_splash,1)
endef
endif

# ---------------------------- PORTS AND DEVICE TREE ---------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_DEVICE_TREE),y)
define RPI_CONFIG_DEVICE_TREE
	$(call enable-option,device_tree,$(BR2_PACKAGE_RPI_CONFIG_DEVICE_TREE_NAME))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_ENABLE_UART),y)
define RPI_CONFIG_ENABLE_UART
	$(call enable-option,enable_uart,1)
endef
endif

# ----------------------------------- GPIOS ------------------------------------

# -------------------------------- OVERCLOCKING --------------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_INITIAL_TURBO),y)
define RPI_CONFIG_INITIAL_TURBO
	$(call enable-option,initial_turbo,1)
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_FORCE_TURBO),y)
define RPI_CONFIG_FORCE_TURBO
	$(call enable-option,force_turbo,1)
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_OVER_VOLTAGE),y)
define RPI_CONFIG_OVER_VOLTAGE
	$(call enable-option,over_voltage,$(BR2_PACKAGE_RPI_CONFIG_OVER_VOLTAGE_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_ARM_FREQ),y)
define RPI_CONFIG_ARM_FREQ
	$(call enable-option,arm_freq,$(BR2_PACKAGE_RPI_CONFIG_ARM_FREQ_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_CORE_FREQ),y)
define RPI_CONFIG_CORE_FREQ
	$(call enable-option,core_freq,$(BR2_PACKAGE_RPI_CONFIG_CORE_FREQ_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_SDRAM_FREQ),y)
define RPI_CONFIG_SDRAM_FREQ
	$(call enable-option,sdram_freq,$(BR2_PACKAGE_RPI_CONFIG_SDRAM_FREQ_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_MINIMAL),y)
define RPI_CONFIG_MINIMAL
	sed -i \
        -e 's/\#.*$$//g' \
        -e '/^$$/d' \
        $(BINARIES_DIR)/rpi-firmware/config.txt
    echo "minimized config.txt"
endef
endif

# -------------------------------- MISCELANEOUS --------------------------------

ifeq ($(BR2_PACKAGE_RPI_CONFIG_AVOID_WARNINGS),y)
define RPI_CONFIG_AVOID_WARNINGS
	$(call enable-option,avoid_warnings,$(BR2_PACKAGE_RPI_CONFIG_AVOID_WARNINGS_VALUE))
endef
endif

ifeq ($(BR2_PACKAGE_RPI_CONFIG_LOGGING_LEVEL),y)
define RPI_CONFIG_LOGGING_LEVEL
	$(call enable-option,logging_level,$(BR2_PACKAGE_RPI_CONFIG_LOGGING_LEVEL_VALUE))
endef
endif

define RPI_CONFIG_INSTALL_IMAGES_CMDS
    printf "\n\n# RPi config\n" >> $(BINARIES_DIR)/rpi-firmware/config.txt

    # Memory
    $(RPI_CONFIG_GPU_MEM)
    $(RPI_CONFIG_GPU_MEM_256)
    $(RPI_CONFIG_GPU_MEM_512)
    $(RPI_CONFIG_GPU_MEM_1024)
    $(RPI_CONFIG_TOTAL_MEM)
    $(RPI_CONFIG_DISABLE_L2CACHE)

    # License Keys/Codec
    $(RPI_CONFIG_DECODE_MPG2)
    $(RPI_CONFIG_DECODE_WVC1)

    # Video/Display
    $(RPI_CONFIG_HDMI_SAFE)
    $(RPI_CONFIG_DISABLE_OVERSCAN)

    # Audio

    # Camera
    $(RPI_CONFIG_DISABLE_CAMERA_LED)

    # Boot
    $(RPI_CONFIG_START_FILE)
    $(RPI_CONFIG_FIXUP_FILE)
    $(RPI_CONFIG_DISABLE_COMMANDLINE_TAGS)
    $(RPI_CONFIG_CMDLINE)
    $(RPI_CONFIG_KERNEL)
    $(RPI_CONFIG_ARM_64BIT)
    $(RPI_CONFIG_ARMSTUB)
    $(RPI_CONFIG_BOOTCODE_DELAY)
    $(RPI_CONFIG_BOOT_DELAY)
    $(RPI_CONFIG_BOOT_DELAY_MS)
    $(RPI_CONFIG_DISABLE_SPLASH)

    # Ports and device tree
    $(RPI_CONFIG_DEVICE_TREE)
    $(RPI_CONFIG_ENABLE_UART)

    # GPIOs

    # Overclocking
    $(RPI_CONFIG_INITIAL_TURBO)
    $(RPI_CONFIG_FORCE_TURBO)
    $(RPI_CONFIG_OVER_VOLTAGE)
    $(RPI_CONFIG_ARM_FREQ)
    $(RPI_CONFIG_CORE_FREQ)
    $(RPI_CONFIG_SDRAM_FREQ)

    # Miscelaneous
    $(RPI_CONFIG_AVOID_WARNINGS)
    $(RPI_CONFIG_LOGGING_LEVEL)

    # Writing to config.txt: done

    $(RPI_CONFIG_MINIMAL)
endef

$(eval $(generic-package))