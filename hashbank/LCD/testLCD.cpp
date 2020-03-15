// TEST
extern "C"{
	#include <pthread.h>
}

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

struct coordinates_touch{
	unsigned int x_pt;
	unsigned int y_pt;
};

struct thread_data {
	int  thread_id;
	TOUCHCTRL *controller;
	coordinates_touch *POINT;
};

void *event_handler_touchctrl(void *threadarg) {
	struct thread_data *data;
	data = (struct thread_data *) threadarg;

	cout << "Thread ID : " << data->thread_id ;
	// cout << " Message : " << data->controller << endl;

	while(1)
	{
		// STOP HERE UNTIL THERE IS INPUT FROM USER
		data->controller->compute_coordinates(); // Read touch screen -- Blocking method
		// Touch points
		data->POINT->x_pt = data->controller->get_coordinatex();
		data->POINT->y_pt = data->controller->get_coordinatey();

		//cout << "x_pt " << x_pt << endl;
		//cout << "y_pt " << y_pt << endl;

		usleep(1000);
	}


	pthread_exit(NULL);
}




int main ()
{

	// SQL database object 
	SQLDB hashbankdb("../cypher.db");

	// SPI pins:
	// MISO, MOSI, CLK, CS -- SPI0

	// Make use of pointers of pointers to not affect LCD while everything else is loading
	// Initialize GPIOS later on (just before initializing LCD)
	GPIOPIN *GPIO87_ptr;
	GPIOPIN **GPIO87_pptr = &GPIO87_ptr;
	GPIOPIN *GPIO23_ptr;
	GPIOPIN **GPIO23_pptr = &GPIO23_ptr;
	// Create GLCD 
	ILI9488 GLCD(GPIO87_pptr,GPIO23_pptr);

	// Touch controller
	GPIOPIN GPIO20("20",true); // IRQ
	TOUCHCTRL tscrn_ctrl(&GPIO20);

	IMG background("background",&hashbankdb,&GLCD,true,false);
	IMG partial_background("partial_background",&hashbankdb,&GLCD,true,false);
	IMG battery_sample("battery_sample",&hashbankdb,&GLCD,true,false);
	// IMG home_icon("home_icon",&hashbankdb,&GLCD,true,false);
	BTN home_button("home_icon", "home_icon_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0);
	//IMG miscellaneous_icon("miscellaneous_icon",&hashbankdb,&GLCD,true,false);
	BTN miscellaneous_icon("miscellaneous_icon", "miscellaneous_icon_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0);
	//IMG settings_icon("settings_icon",&hashbankdb,&GLCD,true,false);
	BTN settings_icon("setting_icon", "setting_icon_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0);
	// IMG favorite_icon("favorite_icon",&hashbankdb,&GLCD,true,false);
	IMG bitcoin_button("bitcoin_button",&hashbankdb,&GLCD,true,false);
	IMG litecoin_button("litecoin_button",&hashbankdb,&GLCD,true,false);
	IMG ethereum_button("ethereum_button",&hashbankdb,&GLCD,true,false);
	// Confirm button
	BTN confirm_button("button", "button_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0);

	FONT montserrat("montserrat",&hashbankdb,&GLCD,0,false);
	// strings
	FONT text_menu(&montserrat,&hashbankdb,&GLCD,0,false);
	FONT text1(&montserrat,&hashbankdb,&GLCD,0,false);
	FONT text2(&montserrat,&hashbankdb,&GLCD,0,false);
	FONT text3(&montserrat,&hashbankdb,&GLCD,0,false);

	// Fonts
	// FONT sans_serif("sserif", &hashbankdb, &GLCD, 0);
	STOPWATCH timer_fps;

	// Start thread
	struct thread_data td;
	struct coordinates_touch pts;
	td.thread_id = 0;
	td.controller = &tscrn_ctrl;
	td.POINT = &pts;
	// Threads
	pthread_t threads;
	int rc = pthread_create(&threads, NULL, event_handler_touchctrl, (void *)&td);
	if (rc) {
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}

	// Create the GPIOs for LCD
	GPIOPIN GPIO23("23"); // RST line/controller (GPIO23)
	GPIOPIN GPIO87("87"); // D/C (Data/Commands) selector (GPIO87)
	// Now set the pointers
	GPIO23_ptr = &GPIO23;
	GPIO87_ptr = &GPIO87;
	// Now initialize LCD
	GLCD.lcd_init();


	string text1_str = "Bitcoin";
	string text2_str = "Litecoin";
	string text3_str = "Ethereum";
	text1.set_text(text1_str,TEXT_LINE_MODE);
	text2.set_text(text2_str,TEXT_LINE_MODE);
	text3.set_text(text3_str,TEXT_LINE_MODE);

	background.render_image(0,0);
	battery_sample.render_image(280,0);
		
	home_button.init(0,64);
	miscellaneous_icon.init(0,130);
	// favorite_icon.render_image(0,196);
	settings_icon.init(0,196);

	string text_menustr = "Dashboard";
	text_menu.set_text(text_menustr,TEXT_LINE_MODE);
	text_menu.render_text(100,30,RGB666_DARKBLUE_COLORLG);

	bitcoin_button.render_image(150,100);
	text1.render_text(140,180,RGB666_WHITE_COLOR);  
	litecoin_button.render_image(150,230);
	text2.render_text(135,310,RGB666_WHITE_COLOR);
	ethereum_button.render_image(150,360);
	text3.render_text(130,440,RGB666_WHITE_COLOR);

	enum MENU_SECTION{MENU_HOME,MENU_SNDRCV,MENU_SETTINGS}; // 0, 1, 2, 3 ...
	int menu_section = 0; // In what section of the menu are we currently in

	do
	{
		// Home
		if( home_button.event_handler(pts.x_pt,pts.y_pt) && menu_section != MENU_HOME )
		{
			partial_background.render_image(65,62); // Clear section of screen

			string text1_str = "Bitcoin";
			string text2_str = "Litecoin";
			string text3_str = "Ethereum";
			text1.set_text(text1_str,TEXT_LINE_MODE);
			text2.set_text(text2_str,TEXT_LINE_MODE);
			text3.set_text(text3_str,TEXT_LINE_MODE);

			bitcoin_button.render_image(150,100);
			text1.render_text(140,180,RGB666_WHITE_COLOR);  
			litecoin_button.render_image(150,230);
			text2.render_text(135,310,RGB666_WHITE_COLOR);
			ethereum_button.render_image(150,360);
			text3.render_text(130,440,RGB666_WHITE_COLOR);

			menu_section = MENU_HOME; // Home
			//continue;
		}
		// Miscellaneous
		if( miscellaneous_icon.event_handler(pts.x_pt,pts.y_pt) && menu_section != MENU_SNDRCV )
		{
			partial_background.render_image(65,62); // Clear section of screen

			string text1_str = "Send Bitcoin";
			string text2_str = "Send Litecoin";
			string text3_str = "Send Ethereum";
			text1.set_text(text1_str,TEXT_LINE_MODE);
			text2.set_text(text2_str,TEXT_LINE_MODE);
			text3.set_text(text3_str,TEXT_LINE_MODE);

			bitcoin_button.render_image(150,100);
			text1.render_text(110,180,RGB666_WHITE_COLOR);  
			litecoin_button.render_image(150,230);
			text2.render_text(110,310,RGB666_WHITE_COLOR);
			ethereum_button.render_image(150,360);
			text3.render_text(110,440,RGB666_WHITE_COLOR);

			menu_section = MENU_SNDRCV; // Send/Receive
			//continue;
		}


		// Settings
		// Currently inside settings
		if( menu_section == MENU_SETTINGS )
		{
			confirm_button.event_handler(pts.x_pt,pts.y_pt);
		}
		if( settings_icon.event_handler(pts.x_pt,pts.y_pt) && menu_section != MENU_SETTINGS )
		{
			partial_background.render_image(65,62); // Clear section of screen
			confirm_button.init(120,360); 
			confirm_button.add_text(&montserrat,"Confirm",16,32,RGB666_WHITE_COLOR); // Add text to button
			menu_section = MENU_SETTINGS; // Setting menu
			//continue;
		}

		usleep(1000);
	}while( 1 );

	pthread_exit(NULL);
	return 0;
	
}

