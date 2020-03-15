#include <iostream>
#include "../libs/FT6236.h"
#include "../libs/GPIO.h"

using namespace std;

int main ()
{
	GPIOPIN GPIO20("20"); // IRQ
	FT6236 TOUCH(&GPIO20);

	while(1)
	{
		TOUCH.getPoint();

		
		usleep(10000);
	}
	
	
	return 0;

}