#ifndef _TOUCHCTRL_H_
#define _TOUCHCTRL_H_

#define TOUCHCTRL_ADDR 0x38

// Registers
#define  DEV_MODE   0x00 
#define  G_MODE     0xA4
#define  TD_STATUS  0x02 
#define  PI_XH      0x03
#define  PI_XL      0x04
#define  PI_YH      0x05
#define  PI_YL      0x06

#include "I2C.h"
#include "GPIO.h"


class TOUCHCTRL {
	public:
		class TS_POINT{
			friend class TOUCHCTRL;
			public:
				// Default constructor
				TS_POINT()
				{
					this->x = 0;
					this->y = 0;
					this->z = 0;
				}
				TS_POINT(int16_t x, int16_t y, int16_t z)
				{
					this->x = x;
					this->y = y;
					this->z = z;
				}
				//Copy constructor
				TS_POINT(const TS_POINT &t) 
				{
					this->x = t.x;
					this->y = t.y;
					this->z = t.z;
				}
				//Assignment operator
				TS_POINT& operator=(const TS_POINT &t)
				{
					this->x = t.x;
					this->y = t.y;
					this->z = t.z;
				} 
  
				bool operator==(TS_POINT);
				bool operator!=(TS_POINT);
			private:
				int16_t x; /*!< X coordinate */
				int16_t y; /*!< Y coordinate */
				int16_t z; /*!< Z coordinate (often used for pressure) */
		};

		TOUCHCTRL(GPIOPIN*);
		// Get touch point
		void compute_coordinates(uint8_t n = 0); // By default just read one single touch point
		void read_data(void);

		TS_POINT touch_point; // Last touch point
		
		unsigned int get_coordinatex(){return touch_point.x;}
		unsigned int get_coordinatey(){return touch_point.y;}

	private:
		// myI2C i2c;
		// INT GPIO
		GPIOPIN *INT_GPIO; // IRQ 

		uint8_t touches; // Number of touch points
		uint16_t touchX[1], touchY[1], touchID[1];

};



#endif