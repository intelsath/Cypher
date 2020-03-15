#include "../libs/GPIO.h"
#include "../libs/PWM.h"

extern "C"{
	#include <inttypes.h>
	#include <unistd.h>
	#include <stdint.h>
}

#include <iostream>
using namespace std;


int main()
{
    string Percent = "20";
	Percent += "0000"; // PWM entries are in nanoseconds, multiplier.
	PWM Backlight("2", "1"); //LCD connected to port 0, channel 0
	Backlight.run("1000000", Percent); // 1kHz signal, Percent duty cycle.

	/*int duty_cycle;
	cout << "Duty Cycle: " << endl;
	cin>>duty_cycle;

	int us_hz1 = 5000;
	int us_hz2 = 3000;

	GPIOPIN GPIO86("86"); // RST line/controller (GPIO23)
	GPIO86.pin_io(PINOUT);

	while(1)
	{
		if( duty_cycle == 0 )
		{
			GPIO86.set_pin(PINHIGH); // HIGH
			usleep(us_hz2);
			GPIO86.set_pin(PINLOW); // LOW
			usleep(us_hz1);
		}
		if( duty_cycle == 1 )
		{
			GPIO86.set_pin(PINHIGH); // HIGH
			usleep(us_hz1);
			GPIO86.set_pin(PINLOW); // LOW
			usleep(us_hz1);
		}

	}*/



	return 0;
}