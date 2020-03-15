// Exit program
#include <iostream>

#include "libs/GPIO.h"

using namespace std;

int main ()
{

	GPIOPIN GPIO26("26"); // GPIO 26 to power off system
	// ADC input pin to detect presence of USB 
	GPIOPIN ADC_4("in_voltage4_raw"); // VIN.USB

	while(1)
	{
		ADC_4.update_adc();
		int adc_val = ADC_4.get_ADCval();
		cout << "ADC4: " << adc_val << endl;

		// Threshold to determine the presence of VIN.USB
		if( adc_val >= 3500 ) 
		{
			
		}



		usleep(10000);


	}


	GPIO26.pin_io(PINOUT);
	GPIO26.set_pin(PINLOW);

	return 0;

}