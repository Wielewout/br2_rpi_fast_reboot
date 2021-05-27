Raspberry Pi

Intro
=====

These instructions apply to the Raspberry Pi Zero W.

How to build it
===============

Configure Buildroot
-------------------

There is a RaspberryPi defconfig file in Buildroot, which you should base your work on:

  $ make rpi0w_defconfig

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
    +-- bcm2708-rpi-zero-w.dtb
    +-- boot.vfat
    +-- rootfs.ext4
    +-- rpi-firmware/
    |   +-- bootcode.bin
    |   +-- cmdline.txt
    |   +-- config.txt
    |   +-- fixup.dat
    |   +-- start.elf
    |   `-- overlays/               [1]
    +-- sdcard.img
    `-- zImage

[1] For the Raspberry Pi Zero W Model (overlay miniuart-bt is needed
    to enable the RPi0W serial console otherwise occupied by the bluetooth
    chip). Alternative would be to disable the serial console in cmdline.txt
    and /etc/inittab.

How to write the SD card
========================

Once the build process is finished you will have an image called "sdcard.img"
in the output/images/ directory.

Copy the bootable "sdcard.img" onto an SD card with "dd":

  $ sudo dd if=output/images/sdcard.img of=/dev/sdX

Insert the SDcard into your Raspberry Pi, and power it up. Your new system
should come up now and start two consoles: one on the serial port on
the P1 header, one on the HDMI output where you can login using a USB
keyboard.
