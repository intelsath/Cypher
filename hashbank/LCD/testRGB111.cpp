// TEST

#include <iostream>

#include "../libs/sqldb.h"
#include "../libs/GPIO.h"
#include "../libs/touchctrl.h"
#include "../libs/ILI9488.h"
#include "../libs/IMG.h"
#include "../libs/BTN.h"
#include "../libs/font.h"
#include "../libs/animation.h"
#include "../libs/stopwatch.h"

using namespace std;



int main ()
{
	GPIOPIN *GPIO87_ptr;
	GPIOPIN **GPIO87_pptr = &GPIO87_ptr;
	GPIOPIN *GPIO23_ptr;
	GPIOPIN **GPIO23_pptr = &GPIO23_ptr;
	// Create GLCD 
	ILI9488 GLCD(GPIO87_pptr,GPIO23_pptr);

	// Create the GPIOs for LCD
	GPIOPIN GPIO23("23"); // RST line/controller (GPIO23)
	GPIOPIN GPIO87("87"); // D/C (Data/Commands) selector (GPIO87)
	// Now set the pointers
	GPIO23_ptr = &GPIO23;
	GPIO87_ptr = &GPIO87;
	// Now initialize LCD
	GLCD.test_splash();


	return 0;

}
