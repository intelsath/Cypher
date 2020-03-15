#include "touchctrl.h"
#include <iostream>

// Constructor
TOUCHCTRL::TOUCHCTRL(GPIOPIN *INT_GPIO)
{
	this->INT_GPIO = INT_GPIO; // IRQ 
	this->INT_GPIO->pin_io(PININ); // INPUT

	//uint16_t y_output = this->i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_XL);

	// std::std::cout << "x_output: " << (int)x_output << std::std::endl;
	//std::std::cout << "y_output: " << x_output << std::std::endl;
}

// Single touch!
void TOUCHCTRL::read_data()
{
	// Detect rising or falling edge
	INT_GPIO->detect_edge();

	// Start 
	I2C i2c("i2c-1");
	i2c.Send_I2C_Byte(TOUCHCTRL_ADDR,DEV_MODE,0x00); // Device mode = normal
	i2c.Send_I2C_Byte(TOUCHCTRL_ADDR,G_MODE,0x00); // Interrupt polling mode

	// Number of touches
	touches = i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,TD_STATUS);
	if( touches > 0 )
	{
		// Touch point -- Read 1 single point [n]
		touchX[0] = i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_XH) & 0x0F;
		touchX[0] <<= 8;
		touchX[0] |= i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_XL); 
		touchY[0] = i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_YH) & 0x0F;
		touchY[0] <<= 8;
		touchY[0] |= i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_YL);
		touchID[0] = i2c.Read_I2C_Byte(TOUCHCTRL_ADDR,PI_YH) >> 4;
	}

}

void TOUCHCTRL::compute_coordinates(uint8_t n) {
	read_data();
	if ((touches == 0) || (n > 1)) {
		touch_point = TS_POINT(1000, 1000, 1000); // Obviously out of bounds
	} else {
		touch_point = TS_POINT(320 - touchX[n], 480 - touchY[n], 1);
	}

	std::cout << "touches: " << (int)touches << std::endl;
	std::cout << "X: " << touch_point.x << std::endl;
	std::cout << "Y: " << touch_point.y << std::endl;
}


  