Instructions Build:
https://forum.digikey.com/t/pocketbeagle-getting-started/567

-------------------------------------------------------------------------------------------------------------------------------
FOR U-BOOT
-------------------------------------------------------------------------------------------------------------------------------
1) Download u-boot, apply patches, distclean and am335x_evm_defconfig with make -DO NOT COMPILE-
 
2) Change bootdelay in configs/am335x_pocketbeagle_defconfig, and in /LinuxBuild/u-boot/.config
+CONFIG_BOOTDELAY=-2 - Note: Use grep -rnw '/home/luis/LinuxBuild/u-boot/' -e 'CONFIG_BOOTDELAY=2' to confirm

3) Change max_hz in /drivers/spi/spi.c (or in /drivers/spi/mxc_spi.c -- Look for it with grep)
max_hz = fdtdec_get_int(blob, node, "spi-max-frequency", 48000000);

4) Change max_spi_bytes in /cmd/spi.c
define MAX_SPI_BYTES 153600	/* Maximum number of bytes we can handle */

5) Change freq to 48mhz in /cmd/spi.c
ret = spi_get_bus_and_cs(bus, cs, 48000000, mode, "spi_generic_drv",str, &dev, &slave);
slave = spi_setup_slave(bus, cs, 48000000, mode);

6) And comment this for loop in same file (/cmd/spi.c):
		/*for (j = 0; j < ((bitlen + 7) / 8); j++)
			printf("%02X", din[j]);
		printf("\n");*/

7) Change this line in /include/configs/am335x_evm.h and add extra commands 
/* Always 128 KiB env size */

#define CONFIG_ENV_SIZE			(1024 << 10)

8) compile with make ARCH=arm CROSS_COMPILE=${CC}

9) Add the bootloader in the SD card/mmc change the count for u-boot.img
sudo dd if=./u-boot/MLO of=${DISK} count=1 seek=1 bs=128k
sudo dd if=./u-boot/u-boot.img of=${DISK} count=4 seek=1 bs=384k


-------------------------------------------------------------------------------------------------------------------------------
FOR KERNEL
-------------------------------------------------------------------------------------------------------------------------------

1) Edit /KERNEL/arch/arm/boot/dts/am335x-pocketbeagle.dts. Change spi-max-frequency = <48000000> to both channels on both spi0 and spi1.

2) Edit /KERNEL/drivers/spi/spidev. Change static unsigned bufsiz = 500000; 

3) Change PIN_OUTPUT_PULLDOWN to PIN_OUTPUT_PULLUP in am335x-pocketbeagle-common.dtsi

/* P1_36 (ZCZ ball A13) ehrpwm0a */
	P1_36_default_pin: pinmux_P1_36_default_pin { pinctrl-single,pins = <
		AM33XX_IOPAD(0x0990, PIN_OUTPUT_PULLUP | INPUT_EN | MUX_MODE1) >; };	/* mcasp0_aclkx.ehrpwm0a */

	P1_36_pwm_pin: pinmux_P1_36_pwm_pin { pinctrl-single,pins = <
		AM33XX_IOPAD(0x0990, PIN_OUTPUT_PULLUP | INPUT_EN | MUX_MODE1)