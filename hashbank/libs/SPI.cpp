#include "SPI.h"

#include <iostream>
#include <cassert>

using namespace std;

SPI::SPI(){
	bits = 8;
}

SPI::~SPI()
{
	spi_free();
}

int SPI::spi_init(const char *device, int mode, int bits, int speed)
{
	// zero fill `struct spi_ioc_transfer xfer[2]`
	memset(&this->xfer, 0, sizeof(this->xfer));

	// open SPIdev
	this->fd = open(device, O_WRONLY);
	if (this->fd < 0)
	{
		printf("error in spi_init(): failed to open the bus \n");
		return SPI_ERR_OPEN;
	}

	// set mode
	printf("Setting mode...\n");
	this->mode = (uint8_t) mode;
	if (ioctl(this->fd, SPI_IOC_WR_MODE, &this->mode) < 0)
	{
		printf("error in spi_init(): can't set bus mode\n");
		return SPI_ERR_SET_MODE;
	}
	
	// set bits per word
	if (bits)
	{
		printf("Setting bits per word...\n");
		this->bits = (uint8_t) bits;
		if (ioctl(this->fd, SPI_IOC_WR_BITS_PER_WORD, &this->bits) < 0)  
		{
			printf("error in spi_init(): can't set bits per word: %s \n",strerror(errno));
			return SPI_ERR_SET_BITS;
		}
	}

	// set max speed [Hz]
	if (speed)
	{
		printf("Setting speed...\n");
		this->speed = (uint32_t) speed;
		if (ioctl(this->fd, SPI_IOC_WR_MAX_SPEED_HZ, &this->speed) < 0)  
		{
			printf("error in spi_init(): can't set max speed [Hz]: %s \n",strerror(errno));
			return SPI_ERR_SET_SPEED;
		}
	}

	/*SPI_DBG("open device='%s' mode=%d bits=%d lsb=%d max_speed=%d [Hz]",
			device, (int)this->mode, (int)this->bits, (int)this->lsb,
			(int)this->speed);*/

	return SPI_ERR_NONE;
}

// read data from SPIdev
int SPI::spi_read(char *rx_buf, int len)
{
  int retv;

  this->xfer.tx_buf = (uint64_t) 0;      // output buffer
  this->xfer.rx_buf = (uint64_t) rx_buf; // input buffer
  this->xfer.len    = (uint32_t) len;    // length of data to read

  retv = ioctl(this->fd, SPI_IOC_MESSAGE(1), &this->xfer);
  if (retv < 0)
  {
    printf("error in spi_read(): ioctl(SPI_IOC_MESSAGE(1)) return %d", retv);
    return SPI_ERR_READ;
  }

  return retv;
}

// write data to SPIdev [word size of 8]
int SPI::spi_write(const char *tx_buf, int len)
{
  int retv;

  this->xfer.tx_buf = (uint64_t) tx_buf; // output buffer
  this->xfer.rx_buf = (uint64_t) 0;      // input buffer
  this->xfer.len    = (uint32_t) len;    // length of data to write
  set_bpw(8);  // Set new bits per word
  //this->xfer.bits_per_word = 8;

  retv = ioctl(this->fd, SPI_IOC_MESSAGE(1), &this->xfer);
  if (retv < 0)
  {
    printf("error in spi_write(): ioctl(SPI_IOC_MESSAGE(1)) return %d", retv);
    return SPI_ERR_WRITE;
  }

  return retv;
}

// write data to SPIdev [word size of 32]
int SPI::spi_write(uint32_t *tx_buf, int len)
{

  int retv;
  this->xfer.tx_buf = (uint64_t) tx_buf; // output buffer
  this->xfer.rx_buf = (uint64_t) 0;      // input buffer
  this->xfer.len    = (uint32_t) len;    // length of data to write
  set_bpw(32);  // Set new bits per word

  retv = ioctl(this->fd, SPI_IOC_MESSAGE(1), &this->xfer);
  if (retv < 0)
  {
    printf("error in spi_write(): ioctl(SPI_IOC_MESSAGE(2)) return %d", retv);
    cout << SPI_ERR_WRITE << endl;
  }

  return retv;
}

// read and write `len` bytes from/to SPIdev
int SPI::spi_exchange(char *rx_buf, const char *tx_buf, int len)
{
  int retv;

  this->xfer.tx_buf = (uint64_t) tx_buf; // output buffer
  this->xfer.rx_buf = (uint64_t) rx_buf; // input buffer
  this->xfer.len    = (uint32_t) len;    // length of data to write

  retv = ioctl(this->fd, SPI_IOC_MESSAGE(1), &this->xfer);
  if (retv < 0)
  {
    printf("error in spi_exchange(): ioctl(SPI_IOC_MESSAGE(1)) return %d",
             retv);
    return SPI_ERR_EXCHANGE;
  }

  return retv;
}

int SPI::set_bpw(int bits)
{

	// set bits per word
	if (this->bits != bits) // If bpw is same as the current one, don't change anything!
	{
		//printf("Setting bits per word...\n");
		this->bits = (uint8_t) bits;
		if (ioctl(this->fd, SPI_IOC_WR_BITS_PER_WORD, &this->bits) < 0)  
		{
			printf("error in spi_write(): can't set bits per word: %s \n",strerror(errno));
			return SPI_ERR_SET_BITS;
		}
		else{
			return 1;
		}
	}
}
// close SPIdev file and free memory
void SPI::spi_free()
{
  close(this->fd);
}