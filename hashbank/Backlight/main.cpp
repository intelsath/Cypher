#include <iostream>
#include "PWM.h"

int main(){
	// Program for brightness
	std::string Percent;
	cin>>Percent;
	Percent = Percent + "0000"; // PWM entries are in nanoseconds, multiplier.
	PWM Brightness("0","0"); //LCD connected to channel 0 on port 0.
	Brightness.run("1000000",Percent); // 1kHz signal, Percent duty cycle.
	return(0);
}
