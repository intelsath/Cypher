#ifndef _I2C_H_
#define _I2C_H_

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>

#define MAX_BUFFER_SIZE	64

class I2C {
	
	public:
		I2C(std::string);	// Constructor
		~I2C();	// Destructor
	
		// Public Variables
		unsigned char I2C_WR_Buf[MAX_BUFFER_SIZE];			// Contains data you want to send
		unsigned char I2C_RD_Buf[MAX_BUFFER_SIZE];			// Contains data which was read
	
		// Initialize Functionsluiis
		void i2cOpen();										
		void i2cClose();								
		void i2cSetAddress(unsigned char address);					// Changes device address
	
		// Sends a single byte <Data> to <DEVICE_ADDR> on the register <Reg_ADDR>
		void Send_I2C_Byte(unsigned char DEVICE_ADDR, unsigned char Reg_ADDR, unsigned char Data);	

		// Reads and returns a single byte from <DEVICE_ADDR> on the register <Reg_ADDR>
		unsigned char Read_I2C_Byte(unsigned char DEVICE_ADDR,unsigned char Reg_ADDR);

	private:
		int g_i2cFile;
		std::string i2c_port; 
};
#endif /* BEAGLEI2C.H */