#ifndef _ILI9488_
#define _ILI9488_

#define ILI9488_TFTWIDTH  320
#define ILI9488_TFTHEIGHT 480

#define ILI9488_NOP     0x00
#define ILI9488_SWRESET 0x01
#define ILI9488_RDDID   0x04
#define ILI9488_RDDST   0x09

#define ILI9488_SLPIN   0x10
#define ILI9488_SLPOUT  0x11
#define ILI9488_PTLON   0x12
#define ILI9488_NORON   0x13

#define ILI9488_RDMODE  0x0A
#define ILI9488_RDMADCTL  0x0B
#define ILI9488_RDPIXFMT  0x0C
#define ILI9488_RDIMGFMT  0x0D
#define ILI9488_RDSELFDIAG  0x0F

#define ILI9488_INVOFF  0x20
#define ILI9488_INVON   0x21
#define ILI9488_GAMMASET 0x26
#define ILI9488_DISPOFF 0x28
#define ILI9488_DISPON  0x29

#define ILI9488_CASET   0x2A
#define ILI9488_PASET   0x2B
#define ILI9488_RAMWR   0x2C
#define ILI9488_RAMRD   0x2E

#define ILI9488_PTLAR   0x30
#define ILI9488_MADCTL  0x36
#define ILI9488_PIXFMT  0x3A

#define ILI9488_FRMCTR1 0xB1
#define ILI9488_FRMCTR2 0xB2
#define ILI9488_FRMCTR3 0xB3
#define ILI9488_INVCTR  0xB4
#define ILI9488_DFUNCTR 0xB6

#define ILI9488_PWCTR1  0xC0
#define ILI9488_PWCTR2  0xC1
#define ILI9488_PWCTR3  0xC2
#define ILI9488_PWCTR4  0xC3
#define ILI9488_PWCTR5  0xC4
#define ILI9488_VMCTR1  0xC5
#define ILI9488_VMCTR2  0xC7

#define ILI9488_RDID1   0xDA
#define ILI9488_RDID2   0xDB
#define ILI9488_RDID3   0xDC
#define ILI9488_RDID4   0xDD

#define ILI9488_GMCTRP1 0xE0
#define ILI9488_GMCTRN1 0xE1

#define ILI9488_CMD_MEMORY_WRITE 0x2C
#define ILI9488_CMD_WRITE_MEMORY_CONTINUE 0x3C
/*
#define ILI9488_PWCTR6  0xFC
*/

#define LCD_SPI_DEVICE "/dev/spidev1.0"

extern "C"{
	#include <inttypes.h>
	#include <unistd.h>
	#include <stdint.h>
	#include "SPI.h"
}
#include <vector>
#include <iostream>
#include "GPIO.h"

class ILI9488{
	private:
		
	public:
		ILI9488(GPIOPIN**,GPIOPIN**); 
		void lcd_init();

		void lcd_sendCmd(uint8_t data); // Send LCD Commands
		void lcd_sendData(uint8_t data);	// Send Data
		void lcd_sendData16(unsigned int data); // Send two bytes of data
		void lcd_sendMultipleBytes(std::vector<uint32_t>); // Send n bytes of data

		// D/C 
		void lcd_dc_low();  
		void lcd_dc_high();
		// RST line
		void lcd_rst_off();
		void lcd_rst_on();

		void lcd_setOrientation(int);
		//void lcd_bg(unsigned int);
		//void lcd_fillRectangle(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
		void lcd_setX(unsigned int,unsigned int);
		void lcd_setY(unsigned int,unsigned int);
		//void lcd_hline(unsigned int, unsigned int, unsigned int, unsigned int);
		void lcd_bmp(std::vector<uint32_t>, unsigned int, unsigned int, unsigned int, unsigned int);

		//void test_splash();

	private:
		int retv; // spi status
		// Use pointers of pointer for these GPIOs -- This is to ensure nothing will afect the LCD (not even the setting of the GPIOS) until it is ready for init()
		GPIOPIN **RSTPIN; // RST line pointer
		GPIOPIN **DCPIN; // D/C line pointer

		unsigned int max_x, max_y;
		unsigned char lcd_orientation;

		// Keep track of the whole "image" being drawn on the screen
		// Map the position in x,y to the exact position of where the pixel is located in a uint32_t vector
		// Keep track of the whole "image" being drawn on the screen
		uint32_t pixel_map[320][480]; // (x,y) -- portrait
		uint32_t BMPpx_map[153600]; // Make an exact copy of the screen and save it on RAM

		SPI spi_bus;

};


#endif