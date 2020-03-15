#ifndef _GPIO_H
#define _GPIO_H

#define PINLOW 0
#define PINHIGH 1
#define PINOUT 0
#define PININ 1

#include <string>

extern "C"
{
	#include <poll.h>
}

class GPIOPIN{
	
	public:
		GPIOPIN(std::string,bool=false);

		int read_agpio(std::string, int*); // Read analogue gpio
		int write_gpio(std::string,std::string); // write to gpio file (value or direction)
		int pin_io(int); //  set THIS pin to either out or in
		// Set pin to either high or low
		int set_pin(int);
		// Enable edge detect (interrupts)
		int enable_interrupt();
		
		// Update ADC value
		void update_adc();
		// Get ADC
		int get_ADCval(){return ADC;}

		// Blocking function to detect a change in the file -- i.e "interrupts"
		int detect_edge();

	private:
		std::string pin_num; // Pin number
		int ADC; // ADC value (if this is an analogue pin)

};

#endif