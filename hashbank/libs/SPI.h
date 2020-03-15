#ifndef _SPI_H
#define _SPI_H
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// common error codes (return values)
#define SPI_ERR_NONE        0 // no error, success
#define SPI_ERR_OPEN       -1 // failed to open the bus
#define SPI_ERR_SET_MODE   -2 // can't set bus mode
#define SPI_ERR_GET_MODE   -3 // can't get bus mode
#define SPI_ERR_GET_LSB    -4 // can't get 'LSB first'
#define SPI_ERR_SET_BITS   -5 // can't set bits per word
#define SPI_ERR_GET_BITS   -6 // can't get bits per word
#define SPI_ERR_SET_SPEED  -7 // can't set max speed [Hz]
#define SPI_ERR_GET_SPEED  -8 // can't get max speed [Hz]
#define SPI_ERR_READ       -9 // can't read
#define SPI_ERR_WRITE     -10 // can't write
#define SPI_ERR_EXCHANGE  -11 // can't read/write

extern "C"
{
	#include <linux/spi/spidev.h>
	#include <stdio.h>
	#include <unistd.h>    // close()
	#include <string.h>    // memset()
	#include <fcntl.h>     // open()
	#include <sys/ioctl.h> // ioctl()
	#include <errno.h> // Error number definitions
	#include <stdint.h>
}

class SPI{
	
	public:
		SPI(); 
		~SPI();

		int spi_init(const char *, int, int, int); // max speed [Hz]

		// close SPIdev file and free memory
		void spi_free();
		//----------------------------------------------------------------------------
		// read data from SPIdev
		int spi_read(char *, int);
		//----------------------------------------------------------------------------
		// write data to SPIdev
		int spi_write(const char *, int );
		int spi_write(uint32_t *, int );
		//----------------------------------------------------------------------------
		// read and write `len` bytes from/to SPIdev
		int spi_exchange(char *, const char *, int);
		// Change bits per word transfer
		int set_bpw(int);

	private:
		int fd;  // file descriptor: fd = open(filename, O_RDWR);
		uint32_t speed; // speed [Hz]
		uint8_t  mode;  // SPI mode
		uint8_t  lsb;   // LSB first
		uint8_t  bits;  // bits per word

		struct spi_ioc_transfer xfer;

};



#endif