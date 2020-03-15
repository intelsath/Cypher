// LSM6DS33 IMU
#include <stdint.h>
#include "IMU.h"

#include "I2C.h"
#include <math.h>
#include <iostream>

IMU::IMU() {    
	//this->i2c = new myI2C();
	// Enable accelerometer
	this->i2c.Send_I2C_Byte(IMU_ADDR,CTRL9_XL,0b00111000); 		
    this->i2c.Send_I2C_Byte(IMU_ADDR,CTRL1_XL, 0b01111100); // Acc = 833Hz (High-Performance mode) : ODR_XL [3:0] 0110 -- Full scale +/- 16G

	// Enable Gyrosensor
	this->i2c.Send_I2C_Byte(IMU_ADDR,CTRL10_C, 0b00111000); // X Y Z access enabled
	this->i2c.Send_I2C_Byte(IMU_ADDR,CTRL2_G, 0b01111100); //  Gyr = 833Hz (High-Performance mode) : ODR_XL [3:0] 0110 -- Full scale 2000dps

    // Initialize variables
    this->samps_iter = 0; // Iterator of array
    this->lastsamples_finished = false; // Init array status

}

// Get the output of any axis from accelerometer
int16_t IMU::get_output_acc(char axis){
	
	int16_t axis_output;
	switch (axis){
		case 'x': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTX_L_XL) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTX_H_XL)<<8));
			break;
		case 'y': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTY_L_XL) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTY_H_XL)<<8));
			break;
		case 'z': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTZ_L_XL) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTZ_H_XL)<<8));
			break;
	}

	return axis_output;
}

// Get the output of any axis from Gyrosensor
int16_t IMU::get_output_gyr(char axis){
	
	int16_t axis_output;
	switch (axis){
		case 'x': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTX_L_G) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTX_H_G)<<8));
			break;
		case 'y': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTY_L_G) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTY_H_G)<<8));
			break;
		case 'z': // Get output of x axis
			axis_output = (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTZ_L_G) | (this->i2c.Read_I2C_Byte(IMU_ADDR,OUTZ_H_G)<<8));
			break;
	}

	return axis_output;
}

// Updates magnitude values
void IMU::update_acc_magnitute(int16_t x, int16_t y, int16_t z)
{
    // Calculate magnitude
    double magnitude = sqrt(pow((double)x/4096,2) + pow((double)y/4096,2) + pow((double)z/4096,2)); // 4096 depends on the scale selection (see datasheet mg/LSB)
    this->magnitude_acc = magnitude;
    
    this->last_samples[this->samps_iter] = this->magnitude_acc; // Updates array to ensure that the last sample is saved
    // Update iterator
    if( this->samps_iter >= 9 ) // maximum samples to store in the array 
    {
        this->samps_iter = 0;

        // Array ready!
        if( this->lastsamples_finished == false ){
            this->lastsamples_finished = true;
        }
    }
    else{
        this->samps_iter++;
     }
}

// Determine if the IMU is in motion
bool IMU::is_moving(){
    // Array (last_samples)not ready -- wait until all data is complete
    if( this->lastsamples_finished == false ){
        return false;
    }
    
    // Calculate the mean
    double sum = 0;
    int total_samples = 10; // Total samples in the array

    for( int i=0; i<total_samples; i++ ){
        sum = sum + this->last_samples[i];
    }
    double mean = sum/(double)total_samples;

    // Calculate standard deviation and determine how "close" most samples are to the mean
    double sigma_sum = 0;
    for( int i=0; i<total_samples; i++ ){
        sigma_sum = sigma_sum + pow(this->last_samples[i] - mean,2);
    }
    double std = sqrt(1/(double)total_samples * sigma_sum);

    // Is the IMU in motion? 
    bool is_moving = false;
    double std_threshold = 0.1; // Minimum "noise" to be considered in motion

    // This is the minimum amount to be considered in motion
    if ( std >= std_threshold){
        is_moving = true;
    }else{
        is_moving = false;
    }

    return is_moving;
}