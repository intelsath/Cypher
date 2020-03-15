#include "I2C.h"
#include <iostream>
using namespace std;

I2C::I2C(string i2c_port){
	this->i2c_port = i2c_port;
	this->i2cOpen();
}

I2C::~I2C(){
	this->i2cClose();
}

// open the Linux device
void I2C::i2cOpen()
{
	string diri2c = "/dev/" + i2c_port;
	// diri2c
	char * cstr_file = new char [diri2c.length()+1];
	std::strcpy (cstr_file, diri2c.c_str()); // Copy string to cstr

	g_i2cFile = open(cstr_file, O_RDWR);

	if (g_i2cFile < 0) {
		perror("i2cOpen in I2C::i2cOpen");
		exit(1);
	}

	delete[] cstr_file;
//	else cout << "OK"<<endl;
}

// close the Linux device
void I2C::i2cClose()
{
	close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
void I2C::i2cSetAddress(unsigned char address)
{
	//cout << "beagle-i2c setting address 0x"<< hex <<(int)address <<"... ";
	if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {
		perror("i2cSetAddress error in I2C::i2cSetAddress");
		exit(1);
	}
//	else cout << "OK" <<endl;
}


void I2C::Send_I2C_Byte(unsigned char DEVICE_ADDR, unsigned char Reg_ADDR, unsigned char Data){
	i2cSetAddress(DEVICE_ADDR);
	//cout << "beagle-i2c writing 0x"<< hex << (int)Data <<" to 0x"<<hex <<(int)DEVICE_ADDR << ", reg 0x" <<hex<<(int)Reg_ADDR <<"... ";
	I2C_WR_Buf[0] = Reg_ADDR;
	I2C_WR_Buf[1] = Data;

	if(write(g_i2cFile, I2C_WR_Buf, 2) != 2) {
		perror("Write Error in I2C::Send_I2C_Byte");
	}
//	else cout << "OK";

}


unsigned char I2C::Read_I2C_Byte(unsigned char DEVICE_ADDR,unsigned char Reg_ADDR){
	I2C_WR_Buf[0] = Reg_ADDR;
	
	i2cSetAddress(DEVICE_ADDR);
	if(write(g_i2cFile, I2C_WR_Buf, 1) != 1) {
		perror("Write Error in I2C::Read_I2C_Byte");
	}
	i2cSetAddress(DEVICE_ADDR);	
	if(read(g_i2cFile, I2C_RD_Buf, 1) !=1){
		perror("Read Error I2C::Read_I2C_Byte");
	}
	
	return I2C_RD_Buf[0];
}
