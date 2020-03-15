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

	// SQL database object 
	SQLDB hashbankdb("../cypher.db");

	// Initialize GPIOS later on (just before initializing LCD)
	GPIOPIN *GPIO87_ptr;
	GPIOPIN **GPIO87_pptr = &GPIO87_ptr;
	GPIOPIN *GPIO23_ptr;
	GPIOPIN **GPIO23_pptr = &GPIO23_ptr;

	// SPI pins:
	// MISO, MOSI, CLK, CS -- SPI0
	// LCD 
	ILI9488 GLCD(GPIO87_pptr,GPIO23_pptr);

	// Animated images
	// ANIMATION pacman("fps_counter", &hashbankdb, &GLCD, 0, 3, 60);
	IMG fpscounter1("fpscounter1",&hashbankdb,&GLCD,true,false); // 1
	IMG fpscounter2("fpscounter2",&hashbankdb,&GLCD,true,false); // 1
	IMG fpscounter3("fpscounter3",&hashbankdb,&GLCD,true,false); // 1

	IMG enemybmp("bmptest",&hashbankdb,&GLCD,true); // test

	// Fonts
	// FONT sans_serif("sserif", &hashbankdb, &GLCD, 0);

	STOPWATCH timer_fps;

	// Create the GPIOs for LCD
	GPIOPIN GPIO23("23"); // RST line/controller (GPIO23)
	GPIOPIN GPIO87("87"); // D/C (Data/Commands) selector (GPIO87)
	// Now set the pointers
	GPIO23_ptr = &GPIO23;
	GPIO87_ptr = &GPIO87;
	// Init 
	GLCD.lcd_init();

	// pacman.init(0,0); // Initial render
	//sans_serif.render_text("FPS Counter",0,0,0xF800);

	double tau; // Time it took to render 1 frame 
	double fps; // Actual fps
	string fps_count; 
	int frame = 1;
	while(1)
	{
		timer_fps.reset(); // Reset to zero
		timer_fps.start(); // Start
		switch(frame){
			case 1:
				fpscounter1.render_image(0,0);
			break;
			case 2:
				fpscounter2.render_image(0,0);
			break;
			case 3:
				fpscounter3.render_image(0,0);
			break;
		}
		// pacman.animate(0,0); // Keep animating

		// BG.render_image(0,0);
		// Title
		//sans_serif.render_text("FPS Counter",0,0,0xF800);
		// get time
		tau = timer_fps.get_epoch();
		fps = (1/tau);
		string fpsstr = to_string(fps);
		cout << "fps : " << fps << " tau_main: " << tau << endl;

		// Show FPS
		//sans_serif.render_text("FPS",0,250,0xF800);
		//sans_serif.render_text(fpsstr,0,300,0x07E0);

		frame++;
		if( frame>3 )
			frame = 1;
	}
	


	return 0;

}
