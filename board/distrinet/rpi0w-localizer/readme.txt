RPi Localizer

Intro
=====

These instructions apply to the Raspberry Pi Zero W equiped with the RPi Localizer package.

How to build it
===============

Configure Buildroot
-------------------

There is a defconfig file in Buildroot, which provides the fast reboot
with the localizer application and measurement software:

  $ make rpi0w_fast_reboot_defconfig

Build the rootfs
----------------

Note: you will need to have access to the network, since Buildroot will
download the packages' sources.

You may now build your rootfs with:

  $ make all

(This may take a while, consider getting yourself a coffee ;-) )

Result of the build
-------------------

After building, you should obtain this tree:

    output/images/
    +-- boot.vfat
    +-- rootfs.ext4
    +-- rpi-firmware/
    |   +-- bootcode.bin
    |   +-- cmdline.txt
    |   +-- config.txt
    |   +-- fixup.dat
    |   +-- start.elf
    |   `-- overlays/
    +-- sdcard.img
    `-- zImage

How to write the SD card
========================

Once the build process is finished you will have an image called "sdcard.img"
in the output/images/ directory.

Copy the bootable "sdcard.img" onto an SD card with "dd":

  $ sudo dd if=output/images/sdcard.img of=/dev/mmcblk0

Insert the SD card into your Raspberry Pi, and power it up. Your new system
should come up now and start the localizer application. It is working if
artefacts are visible at the top of an attached display.
