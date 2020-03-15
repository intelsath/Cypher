#ifndef _IMU_H
#define _IMU_H


// IMU 
#include "I2C.h"

#define IMU_ADDR 0b1101011 // Gyrosensor + Accelerometer address (0b1101011)
// Accelerometer
#define CTRL9_XL 0x18 // To enable axes
#define CTRL1_XL 0x10 // High-performance mode register
#define OUTX_L_XL 0x28 // Acc out X,l
#define OUTX_H_XL 0x29 // Acc out, X,h
#define OUTY_L_XL 0x2A // ACC out, Y,L
#define OUTY_H_XL 0x2B // ACC OUT, Y,h
#define OUTZ_L_XL 0x2C // ACC OUT, Z,l
#define OUTZ_H_XL 0x2D // ACC OUT, Z,h

// Gyrosensor
#define CTRL10_C 0x19 // To enable axes
#define CTRL2_G 0x11 // Gyro high-performance mode
#define OUTX_L_G 0x22 // Gyro OUT X,l
#define OUTX_H_G 0x23 // Gyro OUT X,h
#define OUTY_L_G 0x24 // Gyro OUT Y,l
#define OUTY_H_G 0x25 // Gyro OUT Y,h
#define OUTZ_L_G 0x26 // Gyro OUT Z,l
#define OUTZ_H_G 0x27 // Gyro OUT Z,h

class IMU{
	private: 
		myI2C i2c;

	public:
		IMU();
		int16_t get_output_acc(char axis); // Returns output as a 16 twos complement number ('x','y' or 'z')
		int16_t get_output_gyr(char axis); // Returns output as a 16 twos complement number ('x','y' or 'z')
		void update_acc_magnitute(int16_t x, int16_t y, int16_t z);
		bool is_moving(); // Determine if the IMU is moving at all (based on the changes on the last n samples)
		// Inline functions
		double get_magnacc(){return magnitude_acc;}

	private:
		double magnitude_acc; // Magnitude from Accelerometer sqrt(X^2+Y^2+Z^2)
		double last_samples[10]; // Record last n samples to calculate average movement --- Since sampling rate is around 100Hz (delay_ms(10)) then this will be a whole second
		bool lastsamples_finished; // Is the last samples complete?
		int samps_iter; // Iterator of last_samples array
	
};

#endif