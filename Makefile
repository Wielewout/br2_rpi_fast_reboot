include .env

%:
	$(MAKE) -C $(BUILDROOT_PATH) \
		BR2_EXTERNAL=$(PWD) \
		O=$(OUTPUT_PATH) \
		$(@)