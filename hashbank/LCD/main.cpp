// TEST
extern "C"{
	#include <pthread.h>
}

#include <iostream>
#include <math.h>

#include "../libs/sqldb.h"
#include "../libs/GPIO.h"
#include "../libs/touchctrl.h"
#include "../libs/ILI9488.h"
#include "../libs/IMG.h"
#include "../libs/BTN.h"
#include "../libs/font.h"
#include "../libs/animation.h"
#include "../libs/stopwatch.h"
#include "../libs/PWM.h"

#include "../libs/IPC.h"

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

int vertical_align(int width, double max_width = 186){
	return ((328 - (ceil(width/max_width) * 16)) / 2);
}

int center_text(int width, int max_width = 153){
	return ((153 - width)/2)+167;
}

void *event_handler_touchctrl(void *threadarg) {
	struct thread_data *data;
	data = (struct thread_data *) threadarg;

	cout << "Thread ID : " << data->thread_id << endl;
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

struct loading_controller {
	int  thread_id;
	bool show_loading_wheel; // Show loading wheel?
	int pt_x;
	int pt_y;
	ANIMATION *loading_animation;
};

void *busy_loading_process(void *threadarg) 
{
	struct loading_controller *data;
	data = (struct loading_controller *) threadarg;

	cout << "Thread ID : " << data->thread_id;

	while(1)
	{
		if(data->show_loading_wheel)
		{
			data->loading_animation->animate(data->pt_x,data->pt_y); // Loading animation
			usleep(1000);
		}
		usleep(1000);
	}


	pthread_exit(NULL);
}

// Show what actions is being requested through USB
struct usb_action{
	unsigned int page; // e.g page=1 : create_address, page=2 : backup wallet, etc etc
};

struct USB_controller {
	int  thread_id;
	usb_action *USB_CONTENT;
	vector<IMG> *IMAGES;
	FONT *montserrat;
	SQLDB* hashbankdb;
	loading_controller *LOAD_CTRL;
};

void *USB_commands(void *threadarg) 
{
	struct USB_controller *data;
	data = (struct USB_controller *) threadarg;

	cout << "USB thread ID : " << data->thread_id << endl;

	data->IMAGES->at(24).render_image(0, 0);
	data->hashbankdb->sql_query("SELECT language FROM user_settings where id=1"); // Records returned
	if(data->hashbankdb->sql_vals[0] == "1"){
		data->IMAGES->at(25).render_image(0, 280);
	}else if(data->hashbankdb->sql_vals[0] == "2"){
		data->IMAGES->at(26).render_image(0, 280);
	}
	string message_displayed = ""; 		string command_received;
	while(1)
	{
		string message_displayed = ""; 
		string command_received;
		// Use a pipe to listen for commands
		IPC::ustring command;
		IPC named_pipecmd("USBcommands");
		named_pipecmd.open_fifo('r');
		named_pipecmd.read_fifo(&command);
		named_pipecmd.vect2str(&command_received, command); // ustring -> string
		named_pipecmd.close_fifo();

		data->USB_CONTENT->page = stoi(command_received);
		
		switch(stoi(command_received)){
			case 0:
				data->LOAD_CTRL->show_loading_wheel = false; // Show loading animation!
				data->IMAGES->at(2).render_image(0, 352);
				data->IMAGES->at(2).render_image(0, 368);
				data->IMAGES->at(2).render_image(0, 394);
				break;
			case 1:
				message_displayed = "Updating cypher...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 2:
				message_displayed = "Creating address...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 3:
				message_displayed = "Getting all addresses...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 4:
				message_displayed = "Creating wallet...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 5:
				message_displayed = "Signing transaction...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 6:	
				message_displayed = "Reading NFC...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 7:
				message_displayed = "Writing to NFC card...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
			case 8:
				message_displayed = "Updating Cypher...";
				data->montserrat->set_text(message_displayed, TEXT_LINE_MODE);
				data->montserrat->render_text(((258 - data->montserrat->get_width_pixels())/2) + 31, 394, RGB666_WHITE_COLOR, 289);
				data->LOAD_CTRL->pt_x = 144;
				data->LOAD_CTRL->pt_y = 352;
				data->LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				data->montserrat->set_text("", TEXT_LINE_MODE);
				data->montserrat->render_text(144, 352, RGB666_WHITE_COLOR);
				break;
		}

		usleep(1000);
	}

	pthread_exit(NULL);
}

string keyboard( vector<BTN> *Keyboard_BTN, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, string input_text = ""){

	bool lower_case = true;
	bool upper_case = false;
	bool writing = true; 
	int position = 290;

	IMG keyboard_background("keyboard_background", hashbankdb, GLCD, true, false);
	IMG keyboard_textfield("keyboard_textfield", hashbankdb, GLCD, true, false);
	keyboard_background.set_lcd_orientation(LCD_LANDSCAPE); // Static function, will change ALL image's orientation settings
	keyboard_background.render_image(0, 120);
	keyboard_textfield.render_image(0, 0);
	montserrat->set_text(input_text, TEXT_LINE_MODE);
	montserrat->render_text(16, 30, RGB666_BLACK_COLOR, 450, 90);
	
	// Render Numbers
	Keyboard_BTN->at(0).init(432, 130);
	Keyboard_BTN->at(1).init(8, 130);
	Keyboard_BTN->at(2).init(55, 130);
	Keyboard_BTN->at(3).init(102, 130);
	Keyboard_BTN->at(4).init(149, 130);
	Keyboard_BTN->at(5).init(196, 130);
	Keyboard_BTN->at(6).init(244, 130);
	Keyboard_BTN->at(7).init(291, 130);
	Keyboard_BTN->at(8).init(338, 130);
	Keyboard_BTN->at(9).init(385, 130);


	Keyboard_BTN->at(10).init(8, 244);		//Render shift
	Keyboard_BTN->at(12).init(408, 244);	//Render eraser

	Keyboard_BTN->at(13).init(361, 282);	//Render dot
	Keyboard_BTN->at(14).init(124, 282);	//Render space
	Keyboard_BTN->at(15).init(408, 282);	//Render Enter

	//	INIT lower case
	//Render from q to p
	Keyboard_BTN->at(16).init(8, 168);
	Keyboard_BTN->at(17).init(55, 168);
	Keyboard_BTN->at(18).init(102, 168);
	Keyboard_BTN->at(19).init(149, 168);
	Keyboard_BTN->at(20).init(196, 168);
	Keyboard_BTN->at(21).init(244, 168);
	Keyboard_BTN->at(22).init(291, 168);
	Keyboard_BTN->at(23).init(338, 168);
	Keyboard_BTN->at(24).init(385, 168);
	Keyboard_BTN->at(25).init(432, 168);
	
	// Render from a to l
	Keyboard_BTN->at(26).init(32, 206);
	Keyboard_BTN->at(27).init(79, 206);
	Keyboard_BTN->at(28).init(126, 206);
	Keyboard_BTN->at(29).init(173, 206);
	Keyboard_BTN->at(30).init(220, 206);
	Keyboard_BTN->at(31).init(267, 206);
	Keyboard_BTN->at(32).init(314, 206);
	Keyboard_BTN->at(33).init(361, 206);
	Keyboard_BTN->at(34).init(408, 206);

	// Render form z to m
	Keyboard_BTN->at(35).init(79, 244);
	Keyboard_BTN->at(36).init(126, 244);
	Keyboard_BTN->at(37).init(173, 244);
	Keyboard_BTN->at(38).init(220, 244);
	Keyboard_BTN->at(39).init(267, 244);
	Keyboard_BTN->at(40).init(314, 244);
	Keyboard_BTN->at(41).init(361, 244);
	//

	do{
		if(lower_case){
			if( Keyboard_BTN->at(10).event_handler(480 - pts->y_pt, pts->x_pt) ){
				lower_case = false;
				// INIT upper case
				Keyboard_BTN->at(11).init(8, 244);
				Keyboard_BTN->at(42).init(8, 168);
				Keyboard_BTN->at(43).init(55, 168);
				Keyboard_BTN->at(44).init(102, 168);
				Keyboard_BTN->at(45).init(149, 168);
				Keyboard_BTN->at(46).init(196, 168);
				Keyboard_BTN->at(47).init(244, 168);
				Keyboard_BTN->at(48).init(291, 168);
				Keyboard_BTN->at(49).init(338, 168);
				Keyboard_BTN->at(50).init(385, 168);
				Keyboard_BTN->at(51).init(432, 168);
				
				Keyboard_BTN->at(52).init(32, 206);
				Keyboard_BTN->at(53).init(79, 206);
				Keyboard_BTN->at(54).init(126, 206);
				Keyboard_BTN->at(55).init(173, 206);
				Keyboard_BTN->at(56).init(220, 206);
				Keyboard_BTN->at(57).init(267, 206);
				Keyboard_BTN->at(58).init(314, 206);
				Keyboard_BTN->at(59).init(361, 206);
				Keyboard_BTN->at(60).init(408, 206);

				Keyboard_BTN->at(61).init(79, 244);
				Keyboard_BTN->at(62).init(126, 244);
				Keyboard_BTN->at(63).init(173, 244);
				Keyboard_BTN->at(64).init(220, 244);
				Keyboard_BTN->at(65).init(267, 244);
				Keyboard_BTN->at(66).init(314, 244);
				Keyboard_BTN->at(67).init(361, 244);
				//
			}else if( Keyboard_BTN->at(0).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "0";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);  
			}else if( Keyboard_BTN->at(1).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "1";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(2).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "2";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(3).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "3";
				cout << input_text << endl;			
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(4).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "4";
				cout << input_text << endl;		
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(5).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "5";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(6).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "6";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(7).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "7";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(8).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "8";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(9).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "9";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(14).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += " ";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(12).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text = input_text.substr(0, input_text.size() - 1);
				//keyboard_textfield.render_image(0, 0);
				//montserrat->set_text(input_text, TEXT_LINE_MODE);
				//montserrat->render_text(16, 30, RGB666_BLACK_COLOR);
				montserrat->del_text();
				cout << input_text << endl;
			}else if( Keyboard_BTN->at(26).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "a";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16, 30, RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(39).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "b";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(37).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "c";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(28).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "d";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(18).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "e";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(29).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "f";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(30).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "g";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(31).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "h";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(23).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "i";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(32).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "j";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(33).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "k";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(34).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "l";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(41).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "m";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(40).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "n";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(24).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "o";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(25).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "p";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(16).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "q";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(19).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "r";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(27).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "s";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(20).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "t";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(22).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "u";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(38).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "v";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(17).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "w";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(36).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "x";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(21).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "y";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(35).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "z";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(13).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += ".";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if(Keyboard_BTN->at(15).event_handler(480 - pts->y_pt, pts->x_pt)){
				//The user has finished writing
				writing = false;
				cout << input_text << endl;
				return input_text;
			}
		}else{
			if( Keyboard_BTN->at(11).event_handler(480 - pts->y_pt, pts->x_pt)){
				if (upper_case) {
					upper_case = false;
				render_lower:
					lower_case = true;
					// INIT lower case
					Keyboard_BTN->at(10).init(8, 244);
					Keyboard_BTN->at(16).init(8, 168);
					Keyboard_BTN->at(17).init(55, 168);
					Keyboard_BTN->at(18).init(102, 168);
					Keyboard_BTN->at(19).init(149, 168);
					Keyboard_BTN->at(20).init(196, 168);
					Keyboard_BTN->at(21).init(244, 168);
					Keyboard_BTN->at(22).init(291, 168);
					Keyboard_BTN->at(23).init(338, 168);
					Keyboard_BTN->at(24).init(385, 168);
					Keyboard_BTN->at(25).init(432, 168);

					Keyboard_BTN->at(26).init(32, 206);
					Keyboard_BTN->at(27).init(79, 206);
					Keyboard_BTN->at(28).init(126, 206);
					Keyboard_BTN->at(29).init(173, 206);
					Keyboard_BTN->at(30).init(220, 206);
					Keyboard_BTN->at(31).init(267, 206);
					Keyboard_BTN->at(32).init(314, 206);
					Keyboard_BTN->at(33).init(361, 206);
					Keyboard_BTN->at(34).init(408, 206);

					Keyboard_BTN->at(35).init(79, 244);
					Keyboard_BTN->at(36).init(126, 244);
					Keyboard_BTN->at(37).init(173, 244);
					Keyboard_BTN->at(38).init(220, 244);
					Keyboard_BTN->at(39).init(267, 244);
					Keyboard_BTN->at(40).init(314, 244);
					Keyboard_BTN->at(41).init(361, 244);
				}
				else {
					cout << "Mantener mayusculas" << endl;
					upper_case = true;
				}
				//
			}
			else if( Keyboard_BTN->at(0).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "0";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);  
			}else if( Keyboard_BTN->at(1).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "1";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(2).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "2";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(3).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "3";
				cout << input_text << endl;			
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(4).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "4";
				cout << input_text << endl;		
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(5).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "5";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(6).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "6";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(7).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "7";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(8).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "8";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(9).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += "9";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(14).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text += " ";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(12).event_handler(480 - pts->y_pt, pts->x_pt) ){
				input_text = input_text.substr(0, input_text.size() - 1);
				cout << input_text << endl;
				keyboard_textfield.render_image(0, 0);
				montserrat->set_text(input_text, TEXT_LINE_MODE);
				montserrat->render_text(16, 30, RGB666_BLACK_COLOR);
				//montserrat->del_text();
			}else if( Keyboard_BTN->at(52).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "A";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case) 
					goto render_lower;
					
			}else if( Keyboard_BTN->at(65).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "B";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(63).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "C";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(54).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "D";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(44).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "E";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(55).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "F";
				cout << input_text << endl;
				if (!upper_case)
					goto render_lower;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if( Keyboard_BTN->at(56).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "G";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(57).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "H";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(49).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "I";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(58).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "J";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(59).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "K";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(60).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "L";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(67).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "M";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(66).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "N";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(50).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "O";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(51).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "P";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(42).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "Q";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(45).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "R";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(53).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "S";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(46).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "T";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(48).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "U";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(64).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "V";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(43).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "W";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(62).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "X";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(47).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "Y";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(61).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += "Z";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
				if (!upper_case)
					goto render_lower;
			}else if( Keyboard_BTN->at(13).event_handler(480 - pts->y_pt, pts->x_pt)){
				input_text += ".";
				cout << input_text << endl;
				montserrat->set_text(input_text ,TEXT_LINE_MODE);
				montserrat->render_text(16,30,RGB666_BLACK_COLOR, 450, 90, true);
			}else if(Keyboard_BTN->at(15).event_handler(480 - pts->y_pt, pts->x_pt)){
				//The user has finished writing
				writing = false;
				cout << input_text << endl;
				return input_text;
			}
		}
	}while( writing );
}

bool password_verification(vector<BTN> *Keyboard_BTN, vector<BTN> *BUTTONS, vector<IMG> *IMAGES, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, string coldstorage_password) {
	IMAGES->at(0).render_image(0, 0);
	IMAGES->at(23).render_image(120, 75);
	
	BUTTONS->at(28).add_text(montserrat, language_database->at(26), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(28).init(20, 245);

	BUTTONS->at(24).init(125, 325);

	//Control
	bool exit = false;
	string password = "";
	do {
		if (BUTTONS->at(28).event_handler(pts->x_pt, pts->y_pt)) {
			password = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, password);
			IMAGES->at(0).set_lcd_orientation(LCD_PORTRAIT);
			IMAGES->at(0).render_image(0, 0);
			IMAGES->at(23).render_image(120, 75);
			string asterisk = "";
			for (int i=0; i < password.size(); i++){
				if(i == 15 || password.size()-1 == i){
					asterisk += "*";
					montserrat->set_text(asterisk, TEXT_LINE_MODE);
					break;
				}else{
					asterisk += "* ";
				}
			}
			BUTTONS->at(28).add_text(montserrat, asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(28).init(20, 245);

			BUTTONS->at(24).init(125, 325);
		}else if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
			if(coldstorage_password != password){
				montserrat->set_text(language_database->at(22), TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 205, RGB666_RED_COLOR);
			}else{
				return true;
			}
		}
	} while (!exit);

}

bool pin_code_verification(vector<BTN> *BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* Background, IMG* pin_line, IMG* Text_eraser_partial, IMG* logo) {
	//images 
	
	Background->render_image(0, 0);
	logo->render_image(120, 30);

	int x_coordinate = 65, y_coordinate = 200, spacing = 70, delay = 75000;
	BUTTONS->at(1).init(x_coordinate, y_coordinate);
	BUTTONS->at(2).init(x_coordinate + spacing, y_coordinate);
	BUTTONS->at(3).init(x_coordinate + (spacing * 2), y_coordinate);
	BUTTONS->at(4).init(x_coordinate, y_coordinate + spacing);
	BUTTONS->at(5).init(x_coordinate + spacing, y_coordinate + spacing);
	BUTTONS->at(6).init(x_coordinate + (spacing * 2), y_coordinate + spacing);
	BUTTONS->at(7).init(x_coordinate, y_coordinate + (spacing * 2));
	BUTTONS->at(8).init(x_coordinate + spacing, y_coordinate + (spacing * 2));
	BUTTONS->at(9).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 2));
	BUTTONS->at(0).init(x_coordinate + spacing, y_coordinate + (spacing * 3));
	BUTTONS->at(11).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 3));
	BUTTONS->at(10).init(248, 125);
	pin_line->render_image(80, 170); //80, 170
	
	int pin_x_pt = 93;

	string asterisk = "";
	montserrat->set_text(asterisk,TEXT_LINE_MODE);
	hashbankdb->sql_query("SELECT pin FROM user_settings  where id=1"); // Records returned
	string pin_code = hashbankdb->sql_vals[0], pin_password = "";
    string Mistake_str = language_database->at(12);
    
	bool mistake_made = false;

	do{			   		
		if(BUTTONS->at(0).event_handler(pts->x_pt,pts->y_pt)){	
			if(asterisk.size() < 12){
				if (mistake_made == true) 
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "0";
			}
		}else if(BUTTONS->at(1).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "1";
			}
		}else if(BUTTONS->at(2).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "2";
			}
		}else if(BUTTONS->at(3).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "3";
			}
		}else if(BUTTONS->at(4).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "4";
			}
		}else if(BUTTONS->at(5).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "5";
			}
		}else if(BUTTONS->at(6).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "6";
			}
		}else if(BUTTONS->at(7).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "7";
			}
		}else if(BUTTONS->at(8).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "8";
			}
		}else if(BUTTONS->at(9).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() < 12){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				asterisk += "* ";
				montserrat->set_text(asterisk,TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt,150,RGB666_WHITE_COLOR);  
				pin_password = pin_password + "9";
			}
		}else if(BUTTONS->at(10).event_handler(pts->x_pt,pts->y_pt)){
			if(asterisk.size() > 0){
				if (mistake_made == true)
				{
					Text_eraser_partial->render_image(0, 140);
					mistake_made = false;
				}
				cout << "deleting text.." << endl;
				montserrat->del_text();  
				montserrat->del_text();  
				cout << asterisk << endl;
				asterisk = asterisk.substr(0, asterisk.size()-2);
				cout << asterisk << endl;
				cout << pin_password << endl;				
				pin_password = pin_password.substr(0, pin_password.size()-1);
				cout << pin_password << endl;				
			}
		}else if(BUTTONS->at(11).event_handler(pts->x_pt,pts->y_pt)){
			if(pin_code != pin_password){
				montserrat->clear_text();
				pin_password = "";
				asterisk = "";
				mistake_made = true;
				montserrat->set_text(Mistake_str, TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 140, RGB666_RED_COLOR);
			}else{
				return true;
			}
		}
		usleep(1000);
	}while(1);
}

void create_new_wallet(vector<BTN> *Keyboard_BTN, vector<BTN> *BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* Background, int wallet_type, loading_controller* LOAD_CTRL){
	// Rendering
	Background->render_image(0, 0);
	montserrat->set_text(language_database->at(30), TEXT_LINE_MODE);
	montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 60, RGB666_WHITE_COLOR);
	string new_wallet_name = "", passphrase = "", asterisk = "";

	BUTTONS->at(28).add_text(montserrat, language_database->at(31), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(28).init(20, 100);

	BUTTONS->at(52).add_text(montserrat, language_database->at(10), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(52).init(20, 180);

	BUTTONS->at(24).init(70, 260);
	BUTTONS->at(23).init(180, 260);

	//Control
	bool exit = false, flag = false;
	do {
		if (BUTTONS->at(28).event_handler(pts->x_pt, pts->y_pt)) {
			//Teclado, devuelve texto
			new_wallet_name = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, new_wallet_name);
			Background->set_lcd_orientation(LCD_PORTRAIT);
			Background->render_image(0, 0);

			montserrat->set_text(language_database->at(30), TEXT_LINE_MODE);
			montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 60, RGB666_WHITE_COLOR);

			if(new_wallet_name == ""){
				BUTTONS->at(28).add_text(montserrat, language_database->at(31), 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(28).init(20, 100);
			}else{
				BUTTONS->at(28).add_text(montserrat, new_wallet_name, 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(28).init(20, 100);
			}
			
			if(asterisk == ""){
				BUTTONS->at(52).add_text(montserrat, language_database->at(10), 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(52).init(20, 180);
			}else{
				BUTTONS->at(52).add_text(montserrat, asterisk, 10, 15, RGB666_GRAY_COLORLG);
				BUTTONS->at(52).init(20, 180);
			}

			BUTTONS->at(24).init(70, 260);
			BUTTONS->at(23).init(180, 260);
		}else if (BUTTONS->at(52).event_handler(pts->x_pt, pts->y_pt)) {
			passphrase = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, passphrase);
			Background->set_lcd_orientation(LCD_PORTRAIT);
			Background->render_image(0, 0);

			montserrat->set_text(language_database->at(30), TEXT_LINE_MODE);
			montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 60, RGB666_WHITE_COLOR);

			asterisk = "";
			for (int i=0; i < passphrase.size(); i++){
				if(i == 14 || passphrase.size()-1 == i){
					asterisk += "*";
					montserrat->set_text(asterisk, TEXT_LINE_MODE);
					break;
				}else{
					asterisk += "* ";
				}
			}
			
			if(new_wallet_name == ""){
				BUTTONS->at(28).add_text(montserrat, language_database->at(31), 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(28).init(20, 100);
			}else{
				BUTTONS->at(28).add_text(montserrat, new_wallet_name, 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(28).init(20, 100);
			}
			
			if(asterisk == ""){
				BUTTONS->at(52).add_text(montserrat, language_database->at(10), 10, 12, RGB666_GRAY_COLORLG);
				BUTTONS->at(52).init(20, 180);
			}else{
				BUTTONS->at(52).add_text(montserrat, asterisk, 10, 15, RGB666_GRAY_COLORLG);
				BUTTONS->at(52).init(20, 180);
			}

			BUTTONS->at(24).init(70, 260);
			BUTTONS->at(23).init(180, 260);
		}else if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
			if (new_wallet_name == ""){
				cout << "se quiso mandar vacio" << endl;
				montserrat->set_text("Wallet name cannot", TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 300, RGB666_RED_COLOR);
				montserrat->set_text("be empty", TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 316, RGB666_RED_COLOR);
			}else{
				Background->render_image(0, 0);
				montserrat->set_text(language_database->at(23), TEXT_LINE_MODE);
				montserrat->render_text(((320 - montserrat->get_width_pixels()) / 2), 238, RGB666_WHITE_COLOR);
				string script_cmd = "nodejs ../cryptolibs/create_wallet.js '" + new_wallet_name + "' " + to_string(wallet_type) + " '" + passphrase + "'";
				char * WALLET_CMD = new char[script_cmd.length() + 1];
				strcpy(WALLET_CMD, script_cmd.c_str());
				cout << "running script: " << WALLET_CMD << endl;
				LOAD_CTRL->pt_x = 144;
				LOAD_CTRL->pt_y = 196;
				LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
				montserrat->set_text("", TEXT_LINE_MODE);
				montserrat->render_text(0, 240, RGB666_WHITE_COLOR);
				int script_status = system(WALLET_CMD); // Run script! if returned 0 -- sucess
				// Free memory
				delete[] WALLET_CMD;
				cout << "loading wheel turning off" << endl;
				LOAD_CTRL->show_loading_wheel = false;
				exit = true;
			}
		}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono cancelar" << endl;
			exit = true;
			flag = true;
		}
	} while (!exit);

	if(!flag){
		Background->render_image(0, 0);
		montserrat->set_text(language_database->at(27), TEXT_WALL_MODE);
		
		int text_width = montserrat->get_width_pixels(), pt_y = vertical_align(text_width);
		montserrat->render_text(31, pt_y + 46, RGB666_WHITE_COLOR, 289);

		double max_width = 258;
		
		BUTTONS->at(24).init(60, pt_y + 86 + (ceil(text_width / max_width) * 16));
		BUTTONS->at(23).init(190, pt_y + 86 + (ceil(text_width / max_width) * 16));

		bool second_flag = false;
		exit = false;
		do {
			if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
				string sqlqry  = "SELECT mnemonic,wallet_type FROM wallets WHERE ID = (SELECT MAX(ID) from wallets);";

				// Send command to NFC process
				IPC::ustring nfc_cmd; // ustring to send to pipe
				string command = to_string(1); // NFC command
				IPC named_pipecmd("NFCcommands");
				named_pipecmd.str2vect(&nfc_cmd, command);
				named_pipecmd.open_fifo('w');
				named_pipecmd.write_fifo(nfc_cmd);
				named_pipecmd.close_fifo(); // Close pipe

				int returned_records = hashbankdb->sql_query(sqlqry); // Records returned

				//cout << "returned_records " << returned_records << endl;
				string mnemonic; // mnemonic
				string wallet_type; // Wallet type
				// Retrieve values from db
				if( returned_records > 0 )
				{
					//cout << "Sending to NFC process... " << endl;
					mnemonic = hashbankdb->sql_vals[0];
					wallet_type = hashbankdb->sql_vals[1];

					// Is this a backup wallet? Or a NFC wallet? -- see wallet_types in db
					mnemonic += " :" + wallet_type; // Add wallet_type at the end

					// convert to ustrin
					IPC::ustring mnemonic_wallet;
					IPC named_pipewr("NFCwallet");
					named_pipewr.str2vect(&mnemonic_wallet, mnemonic);

					// ustring to send to pipe
					named_pipewr.open_fifo('w');
					named_pipewr.write_fifo(mnemonic_wallet);
					named_pipewr.close_fifo(); // Close pipe

					// Confirm contents of NFC card!
					// Open pipe
					IPC::ustring nfc_contents;
					IPC pipenfc_contents("NFCcontents");
					pipenfc_contents.open_fifo('r');
					pipenfc_contents.read_fifo(&nfc_contents);
					pipenfc_contents.close_fifo();
					// Str nfc_contents
					string nfc_strcontents;
					pipenfc_contents.vect2str(&nfc_strcontents, nfc_contents); // ustring -> string
					cout << nfc_strcontents << endl;
				}
				exit = true;
			}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt) && second_flag) {
				exit = true;
			}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
				second_flag = true;
				montserrat->set_text(language_database->at(32), TEXT_WALL_MODE);
				montserrat->render_text(31, pt_y + 6 - (ceil(montserrat->get_width_pixels() / max_width) * 16), RGB666_RED_COLOR, 289);
			}
		} while (!exit);
	}
}

void loadPortfolio(FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, BTN* up, BTN* down, IMG* coin1_img, string coin1_str, string percent1_str, string amount1_str, IMG* coin2_img, string coin2_str, string percent2_str, string amount2_str, IMG* coin3_img, string coin3_str, string percent3_str, string amount3_str, IMG* coin4_img, string coin4_str, string percent4_str, string amount4_str){
    coin1_img->render_image(103, 60);
    montserrat->set_text(coin1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),60,RGB666_WHITE_COLOR);
    montserrat->set_text(percent1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),84,RGB666_WHITE_COLOR);
    montserrat->set_text(amount1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),108,RGB666_WHITE_COLOR);
    
    coin2_img->render_image(103, 160);
    montserrat->set_text(coin2_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),160,RGB666_WHITE_COLOR);
    montserrat->set_text(percent2_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),184,RGB666_WHITE_COLOR);
    montserrat->set_text(amount2_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),208,RGB666_WHITE_COLOR);
    
    coin3_img->render_image(103, 260);
    montserrat->set_text(coin3_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),260,RGB666_WHITE_COLOR);
    montserrat->set_text(percent3_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),284,RGB666_WHITE_COLOR);
    montserrat->set_text(amount3_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),308,RGB666_WHITE_COLOR);
    
    coin4_img->render_image(103, 360);
    montserrat->set_text(coin4_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),360,RGB666_WHITE_COLOR);
    montserrat->set_text(percent4_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),384,RGB666_WHITE_COLOR);
    montserrat->set_text(amount4_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),408,RGB666_WHITE_COLOR);
}

void loadPortfolio(FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, BTN* up, BTN* down, IMG* coin1_img, string coin1_str, string percent1_str, string amount1_str){
    coin1_img->render_image(103, 60);
    montserrat->set_text(coin1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),60,RGB666_WHITE_COLOR);
    montserrat->set_text(percent1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),84,RGB666_WHITE_COLOR);
    montserrat->set_text(amount1_str,TEXT_WALL_MODE);
    montserrat->render_text(center_text(montserrat->get_width_pixels()),108,RGB666_WHITE_COLOR);
}

bool Wallet_loadCreateAddress(vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* coin, BTN* yes, BTN* no) {

    montserrat->set_text(language_database->at(1), TEXT_WALL_MODE);
    
    int text_width = montserrat->get_width_pixels(), pt_y = vertical_align(text_width);
    
    coin->render_image(164, pt_y);
	
	montserrat->render_text(103, pt_y + 88, RGB666_WHITE_COLOR, 289);

	double max_width = 186;
	montserrat->set_text(language_database->at(2), TEXT_LINE_MODE);
	no->init(220, pt_y + 112 + (ceil(text_width / max_width) * 16));
	montserrat->set_text(language_database->at(3), TEXT_LINE_MODE);
	yes->init(104, pt_y + 112 + (ceil(text_width / max_width) * 16));

}

void loadWallet(FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, BTN* bitcoin, BTN* bitcoin_cash, BTN* litecoin, BTN* ethereum, BTN* ripple){
	
	string text1_str = "Bitcoin";
	string text2_str = "Bitcoin   Cash";
	string text3_str = "Litecoin";
	string text4_str = "Ethereum";
	string text5_str = "Ripple";

	montserrat->set_text(text1_str,TEXT_WALL_MODE);
	montserrat->render_text(110,115,RGB666_WHITE_COLOR); 
	
	montserrat->set_text(text2_str,TEXT_WALL_MODE);
	montserrat->render_text(220,115,RGB666_WHITE_COLOR);  

	montserrat->set_text(text3_str,TEXT_WALL_MODE);
	montserrat->render_text(104,241,RGB666_WHITE_COLOR);  

	montserrat->set_text(text4_str,TEXT_WALL_MODE);
	montserrat->render_text(209,241,RGB666_WHITE_COLOR);  
	
	montserrat->set_text(text5_str,TEXT_WALL_MODE);
	montserrat->render_text(110,367,RGB666_WHITE_COLOR);  

	bitcoin->init(103, 30);
	bitcoin_cash->init(211, 30);
	litecoin->init(101, 156);
	ethereum->init(211, 156);
	ripple->init(101, 282);
}

void ChangeWallet_loadCreateWallet(vector<BTN> *Keyboard_BTN, vector<BTN> *BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* Background, int wallet_type, int WalletID = 0)
{
	// Rendering
	Background->render_image(0, 0);
	montserrat->set_text(language_database->at(29), TEXT_LINE_MODE);
	montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 60, RGB666_WHITE_COLOR);
	string new_wallet_name = "";
	BUTTONS->at(28).add_text(montserrat, "", 10, 12, RGB666_GRAY_COLORLG);
	hashbankdb->sql_query("SELECT wallet_name FROM wallets where ID = " + to_string(WalletID)); // Records returned
	new_wallet_name = hashbankdb->sql_vals[0];
	montserrat->set_text(new_wallet_name, TEXT_LINE_MODE);
	
	BUTTONS->at(28).init(20, 100);
	BUTTONS->at(24).init(70, 200);
	BUTTONS->at(23).init(180, 200);

	//Control
	bool exit = false;

	do {
		if (BUTTONS->at(28).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono a�adir texto" << endl;

			//Teclado, devuelve texto

			new_wallet_name = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, new_wallet_name);
			cout << endl;
			Background->set_lcd_orientation(LCD_PORTRAIT);
			Background->render_image(0, 0);

		
			montserrat->set_text(language_database->at(29), TEXT_LINE_MODE);
			montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 60, RGB666_WHITE_COLOR);

			montserrat->set_text(new_wallet_name, TEXT_LINE_MODE);
			BUTTONS->at(28).init(20, 100);
			BUTTONS->at(24).init(70, 200);
			BUTTONS->at(23).init(180, 200);
		}
		else if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "A�adir" << endl;
			if (new_wallet_name == ""){
				cout << "se quiso mandar vacio" << endl;
				montserrat->set_text("Wallet name cannot", TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 300, RGB666_RED_COLOR);
				montserrat->set_text("be empty", TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 316, RGB666_RED_COLOR);
			}else{
				hashbankdb->sql_query("UPDATE wallets SET wallet_name = '" + new_wallet_name + "' WHERE ID = " + to_string(WalletID));
				exit = true;
			}
		}
		else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono cancelar" << endl;
			exit = true;
		}
	} while (!exit);
}

int loadChangeWallet(vector<IMG> *IMAGES, vector<BTN> *Keyboard_BTN, vector<BTN> *BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, loading_controller* LOAD_CTRL)
{
reset_loadChangeWallet:
	IMAGES->at(0).render_image(0, 0); // Render Background

	montserrat->set_text(language_database->at(28), TEXT_LINE_MODE);
	montserrat->render_text(36, 20, RGB666_WHITE_COLOR);

	//Render division lines
	for (int i = 0; i < 4; i++){
		IMAGES->at(13).render_image(i * 80, 46);
	}
	for (int i = 0; i < 4; i++){
		IMAGES->at(13).render_image(i * 80, 390);
	}


	BUTTONS->at(27).init(254, 8); // Render Add button
	hashbankdb->sql_query("SELECT ID FROM wallets where wallet_type = 1"); // Get NFC Wallet IDs
	vector <string> NFCWallet_IDs = hashbankdb->sql_vals;
	hashbankdb->sql_query("SELECT wallet_name FROM wallets where wallet_type = 1"); // Get NFC Wallet IDs
	vector <string> NFCWallet_names = hashbankdb->sql_vals;
	hashbankdb->sql_query("SELECT ID FROM wallets where wallet_type = 2"); // Get NFC Wallet IDs
	vector <string> DeviceWallet_IDs = hashbankdb->sql_vals;
	hashbankdb->sql_query("SELECT wallet_name FROM wallets where wallet_type = 2"); // Get NFC Wallet IDs
	vector <string> DeviceWallet_names = hashbankdb->sql_vals;

	//Control variables

	int NFCWallet_number = static_cast<int>(NFCWallet_IDs.size());
	int NFCWallet_lastPageNumber = NFCWallet_number % 4;
	int NFCWallet_Pages = (NFCWallet_number - NFCWallet_lastPageNumber) / 4 + 1;
	if (NFCWallet_lastPageNumber == 0)
		NFCWallet_Pages -= 1;
	int NFCWallet_page = 1;

	int DeviceWallet_number = static_cast<int>(DeviceWallet_IDs.size());
	int DeviceWallet_lastPageNumber = DeviceWallet_number % 4;
	int DeviceWallet_Pages = (DeviceWallet_number - DeviceWallet_lastPageNumber) / 4 + 1;
	if (DeviceWallet_lastPageNumber == 0)
		DeviceWallet_Pages -= 1;
	int DeviceWallet_page = 1;
	
	bool ChangePage = false;
		  

	for (int i = 0; i < min(NFCWallet_number,4); i++)
	{
		montserrat->set_text(NFCWallet_names[i], TEXT_LINE_MODE);
		BUTTONS->at(30 + i).init(36, 100 + i * 60);
	}
	for (int i = 0; i < min(NFCWallet_number, 4);i++){
		IMAGES->at(14).render_image(46, 105 + i * 60);
	}


	BUTTONS->at(21).init(36, 60); // Up arrow
	BUTTONS->at(22).init(36, 350); // Down arrow
	BUTTONS->at(25).init(55, 400, true); // NFC Option
	BUTTONS->at(26).init(215, 400); // Device Option
	montserrat->set_text("NFC", TEXT_LINE_MODE);
	montserrat->render_text(80 - montserrat->get_width_pixels() / 2, 457, RGB666_WHITE_COLOR);
	montserrat->set_text("Device", TEXT_LINE_MODE);
	montserrat->render_text(240 - montserrat->get_width_pixels() / 2, 457, RGB666_WHITE_COLOR);

	// Control Variables
	bool selected_nfcbtn, selected_devicebtn;
	enum WALLET_TYPE { NFC_wallet, DEVICE_wallet };
	int wallet_type = NFC_wallet;
	bool exit = false;

	do {
		if (BUTTONS->at(21).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono arriba" << endl;
			switch (wallet_type) {
			case NFC_wallet:
				if (NFCWallet_page > 1)
					NFCWallet_page -= 1;
				else
					NFCWallet_page = NFCWallet_Pages;
				ChangePage = true;
				break;
			case DEVICE_wallet:
				if (DeviceWallet_page > 1)
					DeviceWallet_page -= 1;
				else
					DeviceWallet_page = DeviceWallet_Pages;
				ChangePage = true;
				break;
			}
		}
		else if (BUTTONS->at(22).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono abajo" << endl;
			switch (wallet_type) {
			case NFC_wallet:
				if (NFCWallet_page == NFCWallet_Pages)
					NFCWallet_page = 1;
				else
					NFCWallet_page += 1;
				ChangePage = true;
				break;
			case DEVICE_wallet:
				if (DeviceWallet_page == DeviceWallet_Pages)
					DeviceWallet_page = 1;
				else
					DeviceWallet_page += 1;
				ChangePage = true;
				break;
			}
		}

		if (ChangePage)
		{
			ChangePage = false;
			
			for (int i = 0; i < 15;i++)
				IMAGES->at(2).render_image(0, 100 + 16 * i);

			switch (wallet_type) {
			case NFC_wallet:
				for (int i = 0; i < min(NFCWallet_number - 4 * (NFCWallet_page - 1), 4); i++)
				{
					montserrat->set_text(NFCWallet_names[i + 4 * (NFCWallet_page - 1)],TEXT_LINE_MODE);
					BUTTONS->at(30 + i).init(36, 100 + i * 60);
				}
				for (int i = 0; i < min(NFCWallet_number - 4 * (NFCWallet_page - 1), 4); i++)
					IMAGES->at(14).render_image(46, 105 + i * 60);

				break;
			case DEVICE_wallet:
				for (int i = 0; i < min(DeviceWallet_number - 4 * (DeviceWallet_page - 1), 4); i++)
				{
					montserrat->set_text(DeviceWallet_names[i + 4 * (DeviceWallet_page - 1)], TEXT_LINE_MODE);
					BUTTONS->at(30 + i).init(36, 100 + i * 60);
				}
				for (int i = 0; i < min(DeviceWallet_number - 4 * (DeviceWallet_page - 1), 4); i++)
					IMAGES->at(15).render_image(46, 105 + i * 60);
				break;
			}

		}

		else if (BUTTONS->at(27).event_handler(pts->x_pt, pts->y_pt)) {
			cout << "Se presiono anadir" << endl;
			create_new_wallet(Keyboard_BTN, BUTTONS, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &(IMAGES->at(0)), wallet_type + 1, LOAD_CTRL); //Since Wallet type must be 1 for NFC and 2 for Device, as opposed to enum.
			goto reset_loadChangeWallet;
		}
		switch (wallet_type) {
		case NFC_wallet:
			selected_nfcbtn = true;
			selected_devicebtn = false;

			for (int i = 0; i < min(NFCWallet_number - 4 * (NFCWallet_page - 1), 4); i++)
			{
				if (BUTTONS->at(30 + i).event_handler(pts->x_pt, pts->y_pt)) {
					cout << "Se presiono NFC " << NFCWallet_IDs[4 * (NFCWallet_page - 1) + i] << endl;
					return stoi(NFCWallet_IDs[4 * (NFCWallet_page - 1) + i]);
					exit = true;
				}
			}
			break;
		case DEVICE_wallet:
			selected_nfcbtn = false;
			selected_devicebtn = true;
			for (int i = 0; i < min(DeviceWallet_number - 4 * (DeviceWallet_page - 1), 4); i++)
			{
				if (BUTTONS->at(30 + i).event_handler(pts->x_pt, pts->y_pt)) {
					cout << "Se presiono device " << DeviceWallet_IDs[4 * (DeviceWallet_page - 1) + i] << endl;
					return stoi(DeviceWallet_IDs[4 * (DeviceWallet_page - 1) + i]);
					exit = true;
				}
			}
			break;
		}
		if (BUTTONS->at(25).event_handler(pts->x_pt, pts->y_pt, selected_nfcbtn) && wallet_type != NFC_wallet) {
			cout << "Se presiono NFC" << endl;
			wallet_type = NFC_wallet;
			for (int i = 0; i < 15;i++)
				IMAGES->at(2).render_image(0, 100 + 16 * i);
			for (int i = 0; i < min(NFCWallet_number, 4); i++)
			{
				montserrat->set_text(NFCWallet_names[i], TEXT_LINE_MODE);
				BUTTONS->at(30 + i).init(36, 100 + i * 60);
			}
			for (int i = 0; i < min(NFCWallet_number, 4); i++)
				IMAGES->at(14).render_image(46, 105 + i * 60);
			

		}
		else if (BUTTONS->at(26).event_handler(pts->x_pt, pts->y_pt, selected_devicebtn) && wallet_type != DEVICE_wallet) {
			cout << "Se presiono Briefcase" << endl;
			wallet_type = DEVICE_wallet;
			for (int i = 0; i < 15;i++)
				IMAGES->at(2).render_image(0, 100 + 16 * i);
			for (int i = 0; i < min(DeviceWallet_number, 4); i++)
			{
				montserrat->set_text(DeviceWallet_names[i], TEXT_LINE_MODE);
				BUTTONS->at(30 + i).init(36, 100 + i * 60);
			}
			for (int i = 0; i < min(DeviceWallet_number, 4);i++) {
				IMAGES->at(15).render_image(46, 105 + i * 60);
			}
		}
	} while (!exit);
}

void Settings_loadCreateBackup(vector<string> *language_database, vector<BTN> *BUTTONS, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts) {
	montserrat->set_text(language_database->at(27), TEXT_WALL_MODE);
    
    int text_width = montserrat->get_width_pixels(), pt_y = vertical_align(text_width);
    
	cout << "Text width: " << text_width << endl;
	cout << "Pt_y: " << pt_y << endl;

	montserrat->render_text(103, pt_y + 46, RGB666_WHITE_COLOR, 289);

	double max_width = 186;
	
	BUTTONS->at(24).init(161, pt_y + 86 + (ceil(text_width / max_width) * 16));
}

bool Settings_Security_loadVerificationPin(vector<BTN> * BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* Background, IMG* pin_line, IMG* Text_eraser_full, IMG* Text_eraser_partial) {

	string Message_str = language_database->at(11);
	string Mistake_str = language_database->at(12);

	montserrat->set_text(Message_str, TEXT_LINE_MODE);
	montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 64, RGB666_WHITE_COLOR);

	int x_coordinate = 65, y_coordinate = 200, spacing = 70, delay = 75000;
	BUTTONS->at(1).init(x_coordinate, y_coordinate);
	BUTTONS->at(2).init(x_coordinate + spacing, y_coordinate);
	BUTTONS->at(3).init(x_coordinate + (spacing * 2), y_coordinate);
	BUTTONS->at(4).init(x_coordinate, y_coordinate + spacing);
	BUTTONS->at(5).init(x_coordinate + spacing, y_coordinate + spacing);
	BUTTONS->at(6).init(x_coordinate + (spacing * 2), y_coordinate + spacing);
	BUTTONS->at(7).init(x_coordinate, y_coordinate + (spacing * 2));
	BUTTONS->at(8).init(x_coordinate + spacing, y_coordinate + (spacing * 2));
	BUTTONS->at(9).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 2));
	BUTTONS->at(0).init(x_coordinate + spacing, y_coordinate + (spacing * 3));
	BUTTONS->at(11).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 3));
	BUTTONS->at(29).init(x_coordinate, y_coordinate + (spacing * 3));
	BUTTONS->at(10).init(245, 125);
	pin_line->render_image(80, 170);

	int pin_x_pt = 93;

	string asterisk = "";
	montserrat->set_text(asterisk, TEXT_LINE_MODE);
	hashbankdb->sql_query("SELECT pin FROM user_settings where id=1"); // Records returned
	string pin_code = hashbankdb->sql_vals[0], pin_password = "";
	int flag = 0;
	// Indicates change pin status. flag = |0: Hasn't used the old pin  |1: has used the old pin but hasn't changed it |2: Changed old pin. 
	int mistake_counter = 0;
	bool flag_cancel = false;
	do {
		if (BUTTONS->at(0).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "0";
			}
		}
		else if (BUTTONS->at(1).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "1";
			}
		}
		else if (BUTTONS->at(2).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "2";
			}
		}
		else if (BUTTONS->at(3).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "3";
			}
		}
		else if (BUTTONS->at(4).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "4";
			}
		}
		else if (BUTTONS->at(5).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "5";
			}
		}
		else if (BUTTONS->at(6).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "6";
			}
		}
		else if (BUTTONS->at(7).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "7";
			}
		}
		else if (BUTTONS->at(8).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "8";
			}
		}
		else if (BUTTONS->at(9).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() < 12) {
				asterisk += "* ";
				montserrat->set_text(asterisk, TEXT_LINE_MODE);
				montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
				pin_password = pin_password + "9";
			}
		}
		else if (BUTTONS->at(10).event_handler(pts->x_pt, pts->y_pt)) {
			if (asterisk.size() > 0) {
				cout << "deleting text.." << endl;
				montserrat->del_text();
				montserrat->del_text();
				cout << asterisk << endl;
				asterisk = asterisk.substr(0, asterisk.size() - 2);
				cout << asterisk << endl;
				cout << pin_password << endl;
				pin_password = pin_password.substr(0, pin_password.size() - 1);
				cout << pin_password << endl;
			}
		}
		else if (BUTTONS->at(11).event_handler(pts->x_pt, pts->y_pt)) {
			if (pin_code != pin_password) {
				if (asterisk.size() > 0)
					montserrat->clear_text();
				pin_password = "";
				asterisk = "";
				if (mistake_counter > 0)
					Text_eraser_full->render_image(0, 96);
				mistake_counter++;
				cout << "Made a mistake" << endl;
				montserrat->set_text(Mistake_str, TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 96, RGB666_RED_COLOR);
				cout << "Rendered Mistake text" << endl;
			}
			else {
				pin_password = "";
				asterisk = "";
				flag = 1;
				if (mistake_counter > 0)
					Text_eraser_full->render_image(0, 96);
				return true;
			}
		}
		else if (BUTTONS->at(29).event_handler(pts->x_pt, pts->y_pt)) {
			return false;
		}
		usleep(1000);
	} while (mistake_counter < 3 && flag != 1 && !flag_cancel);
	return false;
}

bool Settings_Security_loadVerificationPassword(vector<BTN> * BUTTONS, vector<BTN> *Keyboard_BTN, vector<IMG> *IMAGES, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts) {
	IMAGES->at(0).render_image(0,0);
	
	BUTTONS->at(28).add_text(montserrat, language_database->at(26), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(28).init(20, 180);

	BUTTONS->at(24).init(60, 260);
	BUTTONS->at(23).init(190, 260);

	//Control
	bool exit = false;
	hashbankdb->sql_query("SELECT password FROM user_settings  where id=1"); // Records returned
	string coldstorage_password = hashbankdb->sql_vals[0], password = "";
	do {
		if (BUTTONS->at(28).event_handler(pts->x_pt, pts->y_pt)) {
			password = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, password);
			IMAGES->at(0).set_lcd_orientation(LCD_PORTRAIT);
			IMAGES->at(0).render_image(0, 0);
			string asterisk = "";
			for (int i=0; i < password.size(); i++){
				if(i == 15 || password.size()-1 == i){
					asterisk += "*";
					montserrat->set_text(asterisk, TEXT_LINE_MODE);
					break;
				}else{
					asterisk += "* ";
				}
			}
			BUTTONS->at(28).add_text(montserrat, asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(28).init(20, 180);
			
			BUTTONS->at(24).init(60, 260);
			BUTTONS->at(23).init(190, 260);
		}else if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
			if(coldstorage_password != password){
				montserrat->set_text(language_database->at(22), TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 140, RGB666_RED_COLOR);
			}else{
				return true;
			}
		}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
			return false;
		}
	} while (!exit);
}

bool Settings_Security_loadChangePassword(vector<BTN> * BUTTONS, vector<BTN> *Keyboard_BTN, vector<IMG> *IMAGES, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts) {
	IMAGES->at(0).render_image(0,0);
	
	BUTTONS->at(28).add_text(montserrat, language_database->at(24), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(28).init(20, 140);
	
	BUTTONS->at(52).add_text(montserrat, language_database->at(25), 10, 12, RGB666_GRAY_COLORLG);
	BUTTONS->at(52).init(20, 220);

	BUTTONS->at(24).init(60, 300);
	BUTTONS->at(23).init(190, 300);

	//Control
	bool exit = false;
	hashbankdb->sql_query("SELECT password FROM user_settings  where id=1"); // Records returned
	string coldstorage_password = hashbankdb->sql_vals[0], password = "", re_password = "";
	string asterisk = "", re_asterisk = "";
	do {
		if (BUTTONS->at(28).event_handler(pts->x_pt, pts->y_pt)) {
			password = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, password);
			IMAGES->at(0).set_lcd_orientation(LCD_PORTRAIT);
			IMAGES->at(0).render_image(0, 0);
			asterisk = "";
			for (int i=0; i < password.size(); i++){
				if(i == 15 || password.size()-1 == i){
					asterisk += "*";
					break;
				}else{
					asterisk += "* ";
				}
			}
			montserrat->set_text(asterisk, TEXT_LINE_MODE);
			BUTTONS->at(28).add_text(montserrat, asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(28).init(20, 140);

			montserrat->set_text(re_asterisk, TEXT_LINE_MODE);
			BUTTONS->at(52).add_text(montserrat, re_asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(52).init(20, 220);
			
			BUTTONS->at(24).init(60, 300);
			BUTTONS->at(23).init(190, 300);
		}else if (BUTTONS->at(52).event_handler(pts->x_pt, pts->y_pt)) {
			re_password = keyboard(Keyboard_BTN, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, re_password);
			IMAGES->at(0).set_lcd_orientation(LCD_PORTRAIT);
			IMAGES->at(0).render_image(0, 0);
			re_asterisk = "";
			for (int i=0; i < re_password.size(); i++){
				if(i == 15 || re_password.size()-1 == i){
					re_asterisk += "*";
					break;
				}else{
					re_asterisk += "* ";
				}
			}
			montserrat->set_text(asterisk, TEXT_LINE_MODE);
			BUTTONS->at(28).add_text(montserrat, asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(28).init(20, 140);

			montserrat->set_text(re_asterisk, TEXT_LINE_MODE);
			BUTTONS->at(52).add_text(montserrat, re_asterisk, 140 - montserrat->get_width_pixels() / 2, 15, RGB666_GRAY_COLORLG);
			BUTTONS->at(52).init(20, 220);
			
			BUTTONS->at(24).init(60, 300);
			BUTTONS->at(23).init(190, 300);
		}else if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
			if(re_password != password){
				montserrat->set_text(language_database->at(15), TEXT_LINE_MODE);
				montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 272, RGB666_RED_COLOR);
			}else{
				hashbankdb->sql_query("UPDATE user_settings SET password = '" + password + "' WHERE id = 1");
				return true;
			}
		}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
			return false;
		}
	} while (!exit);
}

bool Settings_Security_loadChangePin(vector<BTN> * BUTTONS, vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* Background, IMG* pin_line, IMG* Text_eraser_full, IMG* Text_eraser_partial) {
	Background->render_image(0,0);

	string Message_str, Mistake_str = language_database->at(15);

	montserrat->set_text(Message_str, TEXT_LINE_MODE);
	montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 64, RGB666_WHITE_COLOR);

	int x_coordinate = 65, y_coordinate = 200, spacing = 70, delay = 75000;
	BUTTONS->at(1).init(x_coordinate, y_coordinate);
	BUTTONS->at(2).init(x_coordinate + spacing, y_coordinate);
	BUTTONS->at(3).init(x_coordinate + (spacing * 2), y_coordinate);
	BUTTONS->at(4).init(x_coordinate, y_coordinate + spacing);
	BUTTONS->at(5).init(x_coordinate + spacing, y_coordinate + spacing);
	BUTTONS->at(6).init(x_coordinate + (spacing * 2), y_coordinate + spacing);
	BUTTONS->at(7).init(x_coordinate, y_coordinate + (spacing * 2));
	BUTTONS->at(8).init(x_coordinate + spacing, y_coordinate + (spacing * 2));
	BUTTONS->at(9).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 2));
	BUTTONS->at(0).init(x_coordinate + spacing, y_coordinate + (spacing * 3));
	BUTTONS->at(11).init(x_coordinate + (spacing * 2), y_coordinate + (spacing * 3));
	BUTTONS->at(29).init(x_coordinate, y_coordinate + (spacing * 3));
	BUTTONS->at(10).init(245, 125);
	pin_line->render_image(80, 170);

	int pin_x_pt = 93;
	string asterisk = "";
	montserrat->set_text(asterisk, TEXT_LINE_MODE);
	string pin_code = "", pin_password = "";

	int mistake_counter = 0, flag = 0;

	bool New_pinCheck = false;
		do{
			cout << "flag is: " << flag << endl;
			Text_eraser_full->render_image(0, 64);
			montserrat->clear_text();
			if (New_pinCheck == false)
			{
				Message_str = language_database->at(13);
				if (mistake_counter == 4)
				{
					montserrat->set_text(Mistake_str, TEXT_LINE_MODE);
					montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 96, RGB666_RED_COLOR);
				}
			}
			else if (New_pinCheck == true)
			{
				if (mistake_counter == 4)
				{
					Text_eraser_full->render_image(0, 96);
					mistake_counter = 0;
				}
				Message_str = language_database->at(14);
			}
			
			montserrat->set_text(Message_str, TEXT_LINE_MODE);
			cout << Message_str << ". Width is: " << montserrat->get_width_pixels() << endl << "Rendered in " << 160 - montserrat->get_width_pixels() / 2 << endl;
			montserrat->render_text(160 - montserrat->get_width_pixels() / 2, 64, RGB666_WHITE_COLOR);
			cout << "text rendered" << endl;
			do {
				flag = 1;
				if (BUTTONS->at(0).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "0";
					}
				}
				else if (BUTTONS->at(1).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "1";
					}
				}
				else if (BUTTONS->at(2).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "2";
					}
				}
				else if (BUTTONS->at(3).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "3";
					}
				}
				else if (BUTTONS->at(4).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "4";
					}
				}
				else if (BUTTONS->at(5).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "5";
					}
				}
				else if (BUTTONS->at(6).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "6";
					}
				}
				else if (BUTTONS->at(7).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "7";
					}
				}
				else if (BUTTONS->at(8).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "8";
					}
				}
				else if (BUTTONS->at(9).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() < 12) {
						asterisk += "* ";
						montserrat->set_text(asterisk, TEXT_LINE_MODE);
						montserrat->render_text(pin_x_pt, 150, RGB666_WHITE_COLOR);
						pin_password = pin_password + "9";
					}
				}
				else if (BUTTONS->at(10).event_handler(pts->x_pt, pts->y_pt)) {
					if (asterisk.size() > 0) {
						cout << "deleting text.." << endl;
						montserrat->del_text();
						montserrat->del_text();
						cout << asterisk << endl;
						asterisk = asterisk.substr(0, asterisk.size() - 2);
						cout << asterisk << endl;
						cout << pin_password << endl;
						pin_password = pin_password.substr(0, pin_password.size() - 1);
						cout << pin_password << endl;
					}
				}
				else if (BUTTONS->at(11).event_handler(pts->x_pt, pts->y_pt)) {
					if (New_pinCheck == false)
					{
						montserrat->clear_text();
						asterisk = "";
						pin_code = pin_password;
						New_pinCheck = true;
						cout << "Pin password is: "<< pin_password << endl;
						cout << "Pincode is: " << pin_code << endl;
						pin_password = "";
						flag = 2;
					}
					else{
						cout << "Pincode is: " << pin_code << endl;
						cout << "Password is: " << pin_password << endl;
						if (pin_code != pin_password) {
							mistake_counter = 4;
							montserrat->clear_text();
							pin_password = "";
							asterisk = "";
							New_pinCheck = false;
							cout << "New pin and new new pin do not match" << endl;
							flag = 2;
						}
						else {
							cout << "Yup, changed succesfully" << endl;
							hashbankdb->sql_query("UPDATE user_settings SET pin = '" + pin_code + "' WHERE id = 1");
							flag = 3;
							return true;
						}
					}
				}
				else if (BUTTONS->at(29).event_handler(pts->x_pt, pts->y_pt)) {
					return false;
				}
				usleep(1000);
			} while (flag == 1);
		}while (flag != 3);
}

void loadSettings(vector<string> *language_database, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, IMG* partial_background, BTN* Language, BTN* Brightness, BTN* CreateBackup, BTN* EditWallet, BTN* Security) {
	// Rendering 
	montserrat->set_text(language_database->at(4), TEXT_LINE_MODE);
	Language->init(72, 0);
	montserrat->set_text(language_database->at(5), TEXT_LINE_MODE);
	Brightness->init(72, 60);
	montserrat->set_text(language_database->at(6), TEXT_LINE_MODE);
	CreateBackup->init(72, 120);
	montserrat->set_text(language_database->at(7), TEXT_LINE_MODE);
	EditWallet->init(72, 180);
	montserrat->set_text(language_database->at(8), TEXT_LINE_MODE);
	Security->init(72, 240);
}

bool loadSidebar(int WalletID, vector<IMG> *IMAGES,vector<BTN> *Keyboard_BTN, vector<BTN> *BUTTONS, vector<string> *language_database, PWM* Backlight, int BL_value, FONT* montserrat, SQLDB* hashbankdb, ILI9488* GLCD, TOUCHCTRL* tscrn_ctrl, coordinates_touch* pts, loading_controller* LOAD_CTRL, bool *pin_enable, bool *password_enable) {
	
	/************************ Settings buttons ************************/ 

    BUTTONS->at(38).add_text(montserrat,language_database->at(4),30,22,RGB666_WHITE_COLOR);
    BUTTONS->at(39).add_text(montserrat,language_database->at(5),30,22,RGB666_WHITE_COLOR);
    BUTTONS->at(40).add_text(montserrat,language_database->at(6),30,22,RGB666_WHITE_COLOR);
    BUTTONS->at(41).add_text(montserrat,language_database->at(7),30,22,RGB666_WHITE_COLOR);
    BUTTONS->at(42).add_text(montserrat,language_database->at(8),30,22,RGB666_WHITE_COLOR);
    
    /******************** Settings - language buttons ********************/
 
    BUTTONS->at(43).add_text(montserrat, "English", 30, 22, RGB666_WHITE_COLOR);
    BUTTONS->at(44).add_text(montserrat, "Espanol", 30, 22, RGB666_WHITE_COLOR);
   
	int portfolio_page = 0;
    
    IMAGES->at(4).render_image(0, 0);
    IMAGES->at(5).render_image(5, 20);
    IMAGES->at(6).render_image(72, 0);
    
    BUTTONS->at(12).init(10, 100, true);
    BUTTONS->at(13).init(10, 170);
    BUTTONS->at(14).init(10, 240);
    BUTTONS->at(15).init(10, 410);
    
    BUTTONS->at(21).init(72, 0);
    BUTTONS->at(22).init(72, 440);
    portfolio_page = 1;
    loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(8), "Bitcoin", "16", "0.03455", &IMAGES->at(9), "Bitcoin Cash", "32", "0.23240", &IMAGES->at(10), "Litecoin", "12", "34.23120", &IMAGES->at(11), "Ethereum", "25", "1234.29000");

	bool selected_portfoliobtn, selected_walletbtn, selected_walletchangebtn, selected_settingsbtn; 
	enum MENU_OPTIONS{PORTFOLIO, WALLET, WALLET_CHANGE, SETTINGS}; // 0, 1, 2, 3 ... 
	int menu_chosen = PORTFOLIO;

	enum SUBMENU_WALLET_OPTIONS{HOME_WALLET, CREATE_ADDRESS}; // 0, 1, 2, 3 ... 
	int submenu_wallet_chosen;

	enum SUBMENU_SETTING_OPTIONS{HOME_SETTING, LANGUAGE, BRIGHTNESS, BACKUP, EDIT_WALLET, SECURITY}; // 0, 1, 2, 3 ... 
	int submenu_setting_chosen = HOME_SETTING;

	bool lock = false, exit = false, BL = false;
	string Percent, coinname_wallet_create_address;

	

	do{
		if(BUTTONS->at(12).event_handler(pts->x_pt,pts->y_pt,selected_portfoliobtn)){
            menu_chosen = PORTFOLIO;
            IMAGES->at(6).render_image(72, 0);
            BUTTONS->at(21).init(72, 0);
            BUTTONS->at(22).init(72, 440);
            portfolio_page = 1;
            loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(8), "Bitcoin", "16", "0.03455", &IMAGES->at(9), "Bitcoin Cash", "32", "0.23240", &IMAGES->at(10), "Litecoin", "12", "34.23120", &IMAGES->at(11), "Ethereum", "25", "1234.29000");
        }else if(BUTTONS->at(13).event_handler(pts->x_pt,pts->y_pt,selected_walletbtn)){
			menu_chosen = WALLET;
			submenu_wallet_chosen = HOME_WALLET;
			IMAGES->at(6).render_image(72, 0);
			loadWallet(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(16), &BUTTONS->at(17), &BUTTONS->at(18), &BUTTONS->at(19), &BUTTONS->at(20));	
		}else if(BUTTONS->at(14).event_handler(pts->x_pt,pts->y_pt,selected_walletchangebtn)){
			menu_chosen = WALLET_CHANGE;
			IMAGES->at(6).render_image(72, 0);
			exit = true;
		}else if(BUTTONS->at(15).event_handler(pts->x_pt,pts->y_pt,selected_settingsbtn)){
			menu_chosen = SETTINGS;
			submenu_setting_chosen = HOME_SETTING;
			IMAGES->at(6).render_image(72, 0);
			loadSettings(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(6), &BUTTONS->at(38), &BUTTONS->at(39), &BUTTONS->at(40), &BUTTONS->at(41), &BUTTONS->at(42));
		}
		// Determine which button is selected
		switch(menu_chosen)
		{
			case PORTFOLIO:
				selected_portfoliobtn = true;
				selected_walletbtn = false;
				selected_walletchangebtn = false;
				selected_settingsbtn = false;
                if (BUTTONS->at(21).event_handler(pts->x_pt, pts->y_pt)) {
                    if(portfolio_page > 1){
                        portfolio_page -= 1;
                        switch (portfolio_page){
                            case 1:
                                IMAGES->at(7).render_image(72,40);
                                loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(8), "Bitcoin", "16", "0.03455", &IMAGES->at(9), "Bitcoin Cash", "32", "0.23240", &IMAGES->at(10), "Litecoin", "12", "34.23120", &IMAGES->at(11), "Ethereum", "25", "1234.29000");
                                break;
                            case 2:
                                IMAGES->at(7).render_image(72,40);
                                loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(12), "Ripple", "16", "0.03455");
                                break;
                        }
                    }
                }else if (BUTTONS->at(22).event_handler(pts->x_pt, pts->y_pt)) {
                    if(portfolio_page < 2){   // less than last
                        portfolio_page += 1;
                        switch (portfolio_page){
                            case 1:
                                IMAGES->at(7).render_image(72,40);
                                loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(8), "Bitcoin", "16", "0.03455", &IMAGES->at(9), "Bitcoin Cash", "32", "0.23240", &IMAGES->at(10), "Litecoin", "12", "34.23120", &IMAGES->at(11), "Ethereum", "25", "1234.29000");
                                break;
                            case 2:
                                IMAGES->at(7).render_image(72,40);
                                loadPortfolio(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(21), &BUTTONS->at(22), &IMAGES->at(12), "Ripple", "16", "0.03455");
                                break;
                        }
                    }
                }
			break;
			case WALLET:
				selected_portfoliobtn = false;
				selected_walletbtn = true;
				selected_walletchangebtn = false;
				selected_settingsbtn = false; 
				switch(submenu_wallet_chosen){
					case HOME_WALLET:
						if (BUTTONS->at(16).event_handler(pts->x_pt, pts->y_pt)) {
							coinname_wallet_create_address = "Bitcoin";
							submenu_wallet_chosen = CREATE_ADDRESS;
							IMAGES->at(6).render_image(72, 0);
							Wallet_loadCreateAddress(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(8), &BUTTONS->at(24), &BUTTONS->at(23));
						}
						else if (BUTTONS->at(17).event_handler(pts->x_pt, pts->y_pt)) {
							coinname_wallet_create_address = "BitcoinCash";
							submenu_wallet_chosen = CREATE_ADDRESS;
							IMAGES->at(6).render_image(72, 0);
							Wallet_loadCreateAddress(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(9), &BUTTONS->at(24), &BUTTONS->at(23));
						}
						else if (BUTTONS->at(18).event_handler(pts->x_pt, pts->y_pt)) {
							coinname_wallet_create_address = "Litecoin";
							submenu_wallet_chosen = CREATE_ADDRESS;
							IMAGES->at(6).render_image(72, 0);
							Wallet_loadCreateAddress(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(10), &BUTTONS->at(24), &BUTTONS->at(23));
						}
						else if (BUTTONS->at(19).event_handler(pts->x_pt, pts->y_pt)) {
							coinname_wallet_create_address = "Ethereum";
							submenu_wallet_chosen = CREATE_ADDRESS;
							IMAGES->at(6).render_image(72, 0);
							Wallet_loadCreateAddress(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(11), &BUTTONS->at(24), &BUTTONS->at(23));
						}
						else if (BUTTONS->at(20).event_handler(pts->x_pt, pts->y_pt)) {
							coinname_wallet_create_address = "Ripple";
							submenu_wallet_chosen = CREATE_ADDRESS;
							IMAGES->at(6).render_image(72, 0);
							/*GLCD->test_splash();
							string shut_down_script = "shutdown -P now";
							char * SHTDWN = new char[shut_down_script.length()+1];
							strcpy (SHTDWN, shut_down_script.c_str());
							cout << "shuting down " << endl;
							int script_status = system(SHTDWN); // Run script! if returned 0 -- sucess
							*/
							Wallet_loadCreateAddress(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(12), &BUTTONS->at(24), &BUTTONS->at(23));
						}
						break;
					case CREATE_ADDRESS:
						selected_walletbtn = false;
						if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
							IMAGES->at(6).render_image(72,0);
							montserrat->set_text(language_database->at(21), TEXT_LINE_MODE);
						    montserrat->render_text(((248 - montserrat->get_width_pixels()) / 2) + 72, 258, RGB666_WHITE_COLOR);
							cout << "X point:" << ((248 - montserrat->get_width_pixels()) / 2) + 72 << endl;
							// Run script to create new address
							string script_cmd = "nodejs ../cryptolibs/" + coinname_wallet_create_address + "_generateKey.js " + to_string(WalletID);
							char * COIN_CMD = new char[script_cmd.length()+1];
							strcpy (COIN_CMD, script_cmd.c_str());
							cout << "running script: " << COIN_CMD << endl;
							LOAD_CTRL->pt_x = 180;
							LOAD_CTRL->pt_y = 216;
							LOAD_CTRL->show_loading_wheel = true; // Show loading animation!
							montserrat->set_text("", TEXT_LINE_MODE);
						    montserrat->render_text(0, 240, RGB666_WHITE_COLOR);
							cout << "X point:" << ((248 - montserrat->get_width_pixels()) / 2) + 72 << endl;
							int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess
							// Free memory
							delete[] COIN_CMD;

							
							// SLEEP JUST TO ADD DELAY FOR TESTING ANIMATION 
							//usleep(3000000);
							LOAD_CTRL->show_loading_wheel = false; // Exit loading animation

							if( !script_status ){
								// Addresses
								string public_address;

								string sqlqry  = "SELECT address FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname_wallet_create_address + "') AND wallet_id = " + to_string(WalletID) + " ORDER BY ID DESC LIMIT 1;";
								int returned_records = hashbankdb->sql_query(sqlqry); // Records returned

								// Retrieve values from db
								if( returned_records > 0 ){
									public_address = hashbankdb->sql_vals[0];
									cout << "Public Address: " << public_address << endl;
									cout << "Success" << endl;
								}
							}
							menu_chosen = WALLET;
							submenu_wallet_chosen = HOME_WALLET;
							IMAGES->at(6).render_image(72, 0);
							loadWallet(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(16), &BUTTONS->at(17), &BUTTONS->at(18), &BUTTONS->at(19), &BUTTONS->at(20));
							selected_walletbtn = true;
							BUTTONS->at(13).init(10, 170, selected_walletbtn);
						}else if (BUTTONS->at(23).event_handler(pts->x_pt, pts->y_pt)) {
							menu_chosen = WALLET;
							submenu_wallet_chosen = HOME_WALLET;
							IMAGES->at(6).render_image(72, 0);
							loadWallet(montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &BUTTONS->at(16), &BUTTONS->at(17), &BUTTONS->at(18), &BUTTONS->at(19), &BUTTONS->at(20));
							selected_walletbtn = true;
							BUTTONS->at(13).init(10, 170, selected_walletbtn);
						}
						break; 
				}
			break;
			case WALLET_CHANGE:
				selected_portfoliobtn = false;
				selected_walletbtn = false;
				selected_walletchangebtn = true;
				selected_settingsbtn = false;
				exit = true;
			break;
			case SETTINGS:
				selected_portfoliobtn = false;
				selected_walletbtn = false;
				selected_walletchangebtn = false;
				selected_settingsbtn = true;
				switch(submenu_setting_chosen){
					case HOME_SETTING: 
						if (BUTTONS->at(38).event_handler(pts->x_pt, pts->y_pt)) {
							selected_settingsbtn = false;
                            submenu_setting_chosen = LANGUAGE;
                            IMAGES->at(6).render_image(72, 0);
                            montserrat->set_text("English", TEXT_WALL_MODE);
                            BUTTONS->at(43).init(72, 0);
                            montserrat->set_text("Espanol", TEXT_WALL_MODE);
                            BUTTONS->at(44).init(72, 60);
						}else if (BUTTONS->at(39).event_handler(pts->x_pt, pts->y_pt)) {
							selected_settingsbtn = false;
							submenu_setting_chosen = BRIGHTNESS;
							IMAGES->at(6).render_image(72, 0);
							BUTTONS->at(45).init(120, 330);
							montserrat->set_text(language_database->at(20), TEXT_LINE_MODE);
							montserrat->render_text(200, 352, RGB666_WHITE_COLOR);
							BUTTONS->at(46).init(120, 270);
							montserrat->set_text(language_database->at(19), TEXT_LINE_MODE);
							montserrat->render_text(200, 292, RGB666_WHITE_COLOR);
							BUTTONS->at(47).init(120, 210);
							montserrat->set_text(language_database->at(18), TEXT_LINE_MODE);
							montserrat->render_text(200, 232, RGB666_WHITE_COLOR);
							BUTTONS->at(48).init(120, 150);
							montserrat->set_text(language_database->at(17), TEXT_LINE_MODE);
							montserrat->render_text(200, 172, RGB666_WHITE_COLOR);
							BUTTONS->at(49).init(120, 90);
							montserrat->set_text(language_database->at(16), TEXT_LINE_MODE);
							montserrat->render_text(200, 112, RGB666_WHITE_COLOR);
							if (BL_value < 5) {
								for (int i = 1; i < BL_value;i++) 
									IMAGES->at(16).render_image(120, 330 - i * 60);
								for (int i = BL_value; i < 4;i++) 
									IMAGES->at(17).render_image(120, 330 - i * 60);
								IMAGES->at(18).render_image(120, 90);
							}
							else {
								for (int i = 1; i < 4;i++)
									IMAGES->at(16).render_image(120, 330 - i * 60);
								IMAGES->at(19).render_image(120, 90);
							}

						}else if (BUTTONS->at(40).event_handler(pts->x_pt, pts->y_pt)) {
							selected_settingsbtn = false;
							submenu_setting_chosen = BACKUP;
							IMAGES->at(6).render_image(72, 0);
							Settings_loadCreateBackup(language_database, BUTTONS, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts);
						}else if (BUTTONS->at(41).event_handler(pts->x_pt, pts->y_pt)) {
							selected_settingsbtn = false;
							submenu_setting_chosen = EDIT_WALLET;
							IMAGES->at(6).render_image(72, 0);
						}else if (BUTTONS->at(42).event_handler(pts->x_pt, pts->y_pt)) {
							selected_settingsbtn = false;
                            submenu_setting_chosen = SECURITY;
                            IMAGES->at(6).render_image(72, 0);
							IMAGES->at(20).render_image(72, 0);
							montserrat->set_text(language_database->at(9), TEXT_LINE_MODE);
							montserrat->render_text(102, 22, RGB666_WHITE_COLOR);
							BUTTONS->at(50).init(240, 15);
							if (*pin_enable) {
								IMAGES->at(22).render_image(240, 15);
							}

							IMAGES->at(20).render_image(72, 60);
							montserrat->set_text(language_database->at(10), TEXT_LINE_MODE);
							montserrat->render_text(102, 82, RGB666_WHITE_COLOR);
							BUTTONS->at(51).init(240, 75);
							if (*password_enable) {
								IMAGES->at(22).render_image(240, 75);
							}
						}
						break;
					case LANGUAGE:
						selected_settingsbtn = false;
                        if (BUTTONS->at(43).event_handler(pts->x_pt, pts->y_pt)) {
                            submenu_setting_chosen = HOME_SETTING;
                            hashbankdb->sql_query("UPDATE user_settings SET language = 0 WHERE id = 1");
                            hashbankdb->sql_query("SELECT * FROM language where language_id = 0"); // Records returned
                            *language_database = hashbankdb->sql_vals;
                            IMAGES->at(6).render_image(72, 0);
							BUTTONS->at(15).init(10, 410, true);
							selected_settingsbtn = true;
                            loadSettings(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(6), &BUTTONS->at(38), &BUTTONS->at(39), &BUTTONS->at(40), &BUTTONS->at(41), &BUTTONS->at(42));
                        }else if (BUTTONS->at(44).event_handler(pts->x_pt, pts->y_pt)) {
                            submenu_setting_chosen = HOME_SETTING;
                            hashbankdb->sql_query("UPDATE user_settings SET language = 1 WHERE id = 1");
                            hashbankdb->sql_query("SELECT * FROM language where language_id = 1"); // Records returned
                            *language_database = hashbankdb->sql_vals;
                            IMAGES->at(6).render_image(72, 0);
							BUTTONS->at(15).init(10, 410, true);
							selected_settingsbtn = true;
                            loadSettings(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(6), &BUTTONS->at(38), &BUTTONS->at(39), &BUTTONS->at(40), &BUTTONS->at(41), &BUTTONS->at(42));
                        }
						break;
					case BRIGHTNESS:
						selected_settingsbtn = false;
						if (BUTTONS->at(45).event_handler(pts->x_pt, pts->y_pt)) {
							Percent = "20";
							BL = true;
						}
						else if (BUTTONS->at(46).event_handler(pts->x_pt, pts->y_pt)) {
							Percent = "40";
							BL = true;
						}
						else if (BUTTONS->at(47).event_handler(pts->x_pt, pts->y_pt)) {
							Percent = "60";
							BL = true;
						}
						else if (BUTTONS->at(48).event_handler(pts->x_pt, pts->y_pt)) {
							Percent = "80";
							BL = true;
						}
						else if (BUTTONS->at(49).event_handler(pts->x_pt, pts->y_pt)) {
							Percent = "100";
							BL = true;
						}
						if (BL)
						{
							BL_value = stoi(Percent) / 20;
							if (BL_value < 5) {
								for (int i = 1; i < BL_value;i++)
									IMAGES->at(16).render_image(120, 330 - i * 60);
								for (int i = BL_value; i < 4;i++)
									IMAGES->at(17).render_image(120, 330 - i * 60);
								IMAGES->at(18).render_image(120, 90);
							}
							else {
								for (int i = 1; i < 4;i++)
									IMAGES->at(16).render_image(120, 330 - i * 60);
								IMAGES->at(19).render_image(120, 90);
							}
							hashbankdb->sql_query("UPDATE user_settings SET brightness = '" + Percent + "' WHERE id = 1");
							Percent += "0000"; // PWM entries are in nanoseconds, multiplier.
							cout << "Percent is: " << Percent << endl;
							Backlight->run("1000000", Percent); // 1kHz signal, Percent duty cycle.
							BL = false;
						}
						break;
					case BACKUP:
						selected_settingsbtn = false;
						if (BUTTONS->at(24).event_handler(pts->x_pt, pts->y_pt)) {
							string sqlqry  = "SELECT mnemonic,wallet_type FROM wallets WHERE ID = " + to_string(WalletID) + ";";

							// Send command to NFC process
							IPC::ustring nfc_cmd; // ustring to send to pipe
							string command = to_string(1); // NFC command
							IPC named_pipecmd("NFCcommands");
							named_pipecmd.str2vect(&nfc_cmd, command);
							named_pipecmd.open_fifo('w');
							named_pipecmd.write_fifo(nfc_cmd);
							named_pipecmd.close_fifo(); // Close pipe

							int returned_records = hashbankdb->sql_query(sqlqry); // Records returned

							//cout << "returned_records " << returned_records << endl;
							string mnemonic; // mnemonic
							string wallet_type; // Wallet type
							// Retrieve values from db
							if( returned_records > 0 ){
								//cout << "Sending to NFC process... " << endl;
								mnemonic = hashbankdb->sql_vals[0];
								wallet_type = hashbankdb->sql_vals[1];

								// Is this a backup wallet? Or a NFC wallet? -- see wallet_types in db
								mnemonic += " :" + wallet_type; // Add wallet_type at the end

								// convert to ustrin
								IPC::ustring mnemonic_wallet;
								IPC named_pipewr("NFCwallet");
								named_pipewr.str2vect(&mnemonic_wallet, mnemonic);

								// ustring to send to pipe
								named_pipewr.open_fifo('w');
								named_pipewr.write_fifo(mnemonic_wallet);
								named_pipewr.close_fifo(); // Close pipe

								// Confirm contents of NFC card!
								// Open pipe
								IPC::ustring nfc_contents;
								IPC pipenfc_contents("NFCcontents");
								pipenfc_contents.open_fifo('r');
								pipenfc_contents.read_fifo(&nfc_contents);
								pipenfc_contents.close_fifo();
								// Str nfc_contents
								string nfc_strcontents;
								pipenfc_contents.vect2str(&nfc_strcontents, nfc_contents); // ustring -> string
								cout << nfc_strcontents << endl;
							}
							IMAGES->at(6).render_image(72, 0);
							BUTTONS->at(15).init(10, 410, true);
							selected_settingsbtn = true;
							loadSettings(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(6), &BUTTONS->at(38), &BUTTONS->at(39), &BUTTONS->at(40), &BUTTONS->at(41), &BUTTONS->at(42));
							submenu_setting_chosen = HOME_SETTING;
						}
						break;
					case EDIT_WALLET:
						selected_settingsbtn = false;
						ChangeWallet_loadCreateWallet(Keyboard_BTN, BUTTONS, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &(IMAGES->at(0)), 0, WalletID);
						IMAGES->at(4).render_image(0, 0);
						IMAGES->at(5).render_image(5, 20);
						IMAGES->at(6).render_image(72, 0);

						BUTTONS->at(12).init(10, 100);
						BUTTONS->at(13).init(10, 170);
						BUTTONS->at(14).init(10, 240);
						BUTTONS->at(15).init(10, 410, true);
						submenu_setting_chosen = HOME_SETTING;
						selected_settingsbtn = true;
						loadSettings(language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(6), &BUTTONS->at(38), &BUTTONS->at(39), &BUTTONS->at(40), &BUTTONS->at(41), &BUTTONS->at(42));
					
						break;
					case SECURITY:
						selected_settingsbtn = false;
						if (BUTTONS->at(50).event_handler(pts->x_pt, pts->y_pt)) {
							if (!*pin_enable) {
								*pin_enable = Settings_Security_loadChangePin(BUTTONS, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(0), &IMAGES->at(1), &IMAGES->at(2), &IMAGES->at(3));
								IMAGES->at(0).render_image(0, 0);
								IMAGES->at(4).render_image(0, 0);
								IMAGES->at(5).render_image(5, 20);

								BUTTONS->at(12).init(10, 100);
								BUTTONS->at(13).init(10, 170);
								BUTTONS->at(14).init(10, 240);
								BUTTONS->at(15).init(10, 410);

								submenu_setting_chosen = SECURITY;
								IMAGES->at(20).render_image(72, 0);
								montserrat->set_text(language_database->at(9), TEXT_LINE_MODE);
								montserrat->render_text(102, 22, RGB666_WHITE_COLOR);
								BUTTONS->at(50).init(240, 15);
								if (*pin_enable) {
									IMAGES->at(22).render_image(240, 15);
								}

								IMAGES->at(20).render_image(72, 60);
								montserrat->set_text(language_database->at(10), TEXT_LINE_MODE);
								montserrat->render_text(102, 82, RGB666_WHITE_COLOR);
								BUTTONS->at(51).init(240, 75);
								if (*password_enable) {
									IMAGES->at(22).render_image(240, 75);
								}
							}else {
								if (*password_enable) {
									IMAGES->at(0).render_image(0, 0);
									lock = Settings_Security_loadVerificationPin(BUTTONS, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts, &IMAGES->at(0), &IMAGES->at(1), &IMAGES->at(2), &IMAGES->at(3));
									if (lock) {
										*pin_enable = false;
										hashbankdb->sql_query("UPDATE user_settings SET pin = '' WHERE id = 1");
									}
									IMAGES->at(0).render_image(0, 0);
									IMAGES->at(4).render_image(0, 0);
									IMAGES->at(5).render_image(5, 20);

									BUTTONS->at(12).init(10, 100);
									BUTTONS->at(13).init(10, 170);
									BUTTONS->at(14).init(10, 240);
									BUTTONS->at(15).init(10, 410);

									submenu_setting_chosen = SECURITY;
									IMAGES->at(20).render_image(72, 0);
									montserrat->set_text(language_database->at(9), TEXT_LINE_MODE);
									montserrat->render_text(102, 22, RGB666_WHITE_COLOR);
									BUTTONS->at(50).init(240, 15);
									if (*pin_enable) {
										IMAGES->at(22).render_image(240, 15);
									}

									IMAGES->at(20).render_image(72, 60);
									montserrat->set_text(language_database->at(10), TEXT_LINE_MODE);
									montserrat->render_text(102, 82, RGB666_WHITE_COLOR);
									BUTTONS->at(51).init(240, 75);
									if (*password_enable) {
										IMAGES->at(22).render_image(240, 75);
									}
								}
								else {
									cout << "Tiene que haber un metodo de seguridad activado" << endl;
								}
							}
						}else if (BUTTONS->at(51).event_handler(pts->x_pt, pts->y_pt)) {
							if (!*password_enable) {
								*password_enable = Settings_Security_loadChangePassword(BUTTONS, Keyboard_BTN, IMAGES, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts);
								IMAGES->at(0).render_image(0, 0);
								IMAGES->at(4).render_image(0, 0);
								IMAGES->at(5).render_image(5, 20);

								BUTTONS->at(12).init(10, 100);
								BUTTONS->at(13).init(10, 170);
								BUTTONS->at(14).init(10, 240);
								BUTTONS->at(15).init(10, 410);

								submenu_setting_chosen = SECURITY;
								IMAGES->at(20).render_image(72, 0);
								montserrat->set_text(language_database->at(9), TEXT_LINE_MODE);
								montserrat->render_text(102, 22, RGB666_WHITE_COLOR);
								BUTTONS->at(50).init(240, 15);
								if (*pin_enable) {
									IMAGES->at(22).render_image(240, 15);
								}

								IMAGES->at(20).render_image(72, 60);
								montserrat->set_text(language_database->at(10), TEXT_LINE_MODE);
								montserrat->render_text(102, 82, RGB666_WHITE_COLOR);
								BUTTONS->at(51).init(240, 75);
								if (*password_enable) {
									IMAGES->at(22).render_image(240, 75);
								}
							}
							else {
								if (*pin_enable) {
									IMAGES->at(0).render_image(0, 0);
									lock = Settings_Security_loadVerificationPassword(BUTTONS, Keyboard_BTN, IMAGES, language_database, montserrat, hashbankdb, GLCD, tscrn_ctrl, pts);
									if (lock) {
										*password_enable = false;
										hashbankdb->sql_query("UPDATE user_settings SET password = '' WHERE id = 1");
									}
									IMAGES->at(0).render_image(0, 0);
									IMAGES->at(4).render_image(0, 0);
									IMAGES->at(5).render_image(5, 20);

									BUTTONS->at(12).init(10, 100);
									BUTTONS->at(13).init(10, 170);
									BUTTONS->at(14).init(10, 240);
									BUTTONS->at(15).init(10, 410);

									submenu_setting_chosen = SECURITY;
									IMAGES->at(20).render_image(72, 0);
									montserrat->set_text(language_database->at(9), TEXT_LINE_MODE);
									montserrat->render_text(102, 22, RGB666_WHITE_COLOR);
									BUTTONS->at(50).init(240, 15);
									if (*pin_enable) {
										IMAGES->at(22).render_image(240, 15);
									}

									IMAGES->at(20).render_image(72, 60);
									montserrat->set_text(language_database->at(10), TEXT_LINE_MODE);
									montserrat->render_text(102, 82, RGB666_WHITE_COLOR);
									BUTTONS->at(51).init(240, 75);
									if (*password_enable) {
										IMAGES->at(22).render_image(240, 75);
									}
								}else {
									cout << "Tiene que haber un metodo de seguridad activado" << endl;
								}
							}
						}
						break;
				}
			break;
			
		}
	}while(!exit);
	return false;
}

int main ()
{

	// SQL database object 
	SQLDB hashbankdb("../cypher.db");

	// SPI pins:
	// MISO, MOSI, CLK, CS -- SPI0

	// Make use of pointers of pointers to not affect LCD while everything else is loading
	// Initialize GPIOS later on (just before initializing LCD)
	GPIOPIN *GPIO26_ptr;
	GPIOPIN **GPIO26_pptr = &GPIO26_ptr;
	GPIOPIN *GPIO89_ptr;
	GPIOPIN **GPIO89_pptr = &GPIO89_ptr;
	//Check if SPI available
	std::string dir = "/dev/spidev1.0";
	char * SPI_dir = new char [dir.length()+1];		// Casting literal string to char*
	std::strcpy (SPI_dir, dir.c_str());
	int SPI_ = -1;
	while(SPI_ == -1)						// If it isn't, check until it is.
	{
		cout<<"SPI not available... yet."<<endl;
		SPI_ = open(SPI_dir, O_RDONLY);			// Tries to open file, if it exists.
		usleep(1000);
	}
	cout<<"SPI available"<<endl;
	delete[] SPI_dir;
	close(SPI_);
	
	// Create GLCD 
	ILI9488 GLCD(GPIO26_pptr,GPIO89_pptr);

	// Touch controller
	GPIOPIN GPIO87("87",true); // IRQ
	TOUCHCTRL tscrn_ctrl(&GPIO87);

	//IMG splash_screen("splash_screen", &hashbankdb, &GLCD, true, false);
	//splash_screen.render_image(0,0);

	// Fonts and strings
	FONT montserrat("montserrat", &hashbankdb, &GLCD,false, true);
	
	STOPWATCH timer_fps;

	// Create the GPIOs for LCD
	GPIOPIN GPIO26("26"); // D/C (Data/Commands) selector (GPIO26)
	GPIOPIN GPIO89("89"); // RST line/controller (GPIO89)
	// Now set the pointers
	GPIO89_ptr = &GPIO89;
	GPIO26_ptr = &GPIO26;
	// Now initialize LCD
	
	
	/* LOADING ANIMATION */
	ANIMATION loading_wheel("loading", &hashbankdb, &GLCD, 0, 8, 8);

	// Start thread ---- Touch Controller
	struct thread_data td;
	struct coordinates_touch pts;
	td.thread_id = 0;
	td.controller = &tscrn_ctrl;
	td.POINT = &pts;
	// Threads
	pthread_t thread_touch;
	int rc = pthread_create(&thread_touch, NULL, event_handler_touchctrl, (void *)&td);
	if (rc) {
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}

	// Start thread ---- Loading controller partial background
	struct loading_controller ld;
	ld.thread_id = 1;
	ld.pt_x = 180;
	ld.pt_y = 216;
	ld.show_loading_wheel = false;
	ld.loading_animation = &loading_wheel;
	// Threads
	pthread_t thread_loading;
	rc = pthread_create(&thread_loading, NULL, busy_loading_process, (void *)&ld);
	if (rc) {
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}

	/******************************** Initialization variables *********************************/
	vector<IMG> IMAGES;
	
	IMG Background("background", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(Background); // index 0
	IMG pin_line("pin_line", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(pin_line); // index 1
	IMG Text_eraser_full("Text_eraser_full", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(Text_eraser_full); // index 2
	IMG Text_eraser_partial("Text_eraser_partial", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(Text_eraser_partial); // index 3

	IMG sidebar("sidebar", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(sidebar); // index 4
	IMG sidebar_logo("sidebar_logo", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(sidebar_logo); // index 5
	IMG partial_background("partial_background", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(partial_background); // index 6
    IMG portfolio_partial_background("portfolio_partial_background", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(portfolio_partial_background); // index 7
	IMG bitcoin_img("bitcoin", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(bitcoin_img); // index 8
	IMG bitcoin_cash_img("bitcoin_cash", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(bitcoin_cash_img); // index 9
	IMG litecoin_img("litecoin", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(litecoin_img); // index 10
	IMG ethereum_img("ethereum", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(ethereum_img); // index 11
	IMG ripple_img("ripple", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(ripple_img); // index 12

	IMG line("line", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(line); // index 13
	IMG Wallet_NFC("nfc", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(Wallet_NFC); // index 14
	IMG Wallet_briefcase("briefcase", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(Wallet_briefcase); // index 15

	IMG BL_FILL("BL_btn_2", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(BL_FILL); // index 16
	IMG BL_CLEAR("BL_btn_1", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(BL_CLEAR); // index 17
	IMG BL_CLEART("BL_btnTop_1", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(BL_CLEART); // index 18
	IMG BL_FILLT("BL_btnTop_2", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(BL_FILLT); // index 19
	
	IMG submenu_image("submenu_button", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(submenu_image); // index 20

	IMG slider_img1("slider_security_1", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(slider_img1); // index 21
	IMG slider_img2("slider_security_2", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(slider_img2); // index 22
	IMG logo("logo", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(logo); // index 23
	IMG usb("usb", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(usb); // index 24
	IMG usb_text_eng("usb_text_eng", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(usb_text_eng); // index 25
	IMG usb_text_esp("usb_text_esp", &hashbankdb, &GLCD, true, false);
	IMAGES.push_back(usb_text_esp); // index 26

	vector<BTN> BUTTONS;
	BTN PinNumber0("PinNumber0_1", "PinNumber0_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber0); // index 0
	BTN PinNumber1("PinNumber1_1", "PinNumber1_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber1); // index 1
	BTN PinNumber2("PinNumber2_1", "PinNumber2_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber2); // index 2
	BTN PinNumber3("PinNumber3_1", "PinNumber3_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber3); // index 3
	BTN PinNumber4("PinNumber4_1", "PinNumber4_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber4); // index 4
	BTN PinNumber5("PinNumber5_1", "PinNumber5_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber5); // index 5
	BTN PinNumber6("PinNumber6_1", "PinNumber6_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber6); // index 6
	BTN PinNumber7("PinNumber7_1", "PinNumber7_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber7); // index 7
	BTN PinNumber8("PinNumber8_1", "PinNumber8_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber8); // index 8
	BTN PinNumber9("PinNumber9_1", "PinNumber9_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumber9); // index 9
	
	BTN PinNumberErase("PinNumberErase_1", "PinNumberErase_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumberErase); // index 10
	BTN PinNumberCheck("PinNumberCheck_1", "PinNumberCheck_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumberCheck); // index 11


	BTN portfolio("portfolio_1", "portfolio_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(portfolio); // index 12
	BTN wallet("wallet_1", "wallet_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(wallet); // index 13
	BTN wallet_change("wallet_change_1", "wallet_change_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(wallet_change); // index 14
	BTN settings("settings_1", "settings_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(settings); // index 15


	BTN bitcoin_btn("bitcoin", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(bitcoin_btn); // index 16
	BTN bitcoin_cash_btn("bitcoin_cash", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(bitcoin_cash_btn); // index 17
	BTN litecoin_btn("litecoin", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(litecoin_btn); // index 18
	BTN ethereum_btn("ethereum", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(ethereum_btn); // index 19
	BTN ripple_btn("ripple", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(ripple_btn); // index 20


	BTN up("up_1", "up_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(up); // index 21
	BTN down("down_1", "down_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
    BUTTONS.push_back(down); // index 22


	BTN confirm_neg("confirm_button_neg", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(confirm_neg); // index 23
	BTN confirm_pos("confirm_button_pos", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(confirm_pos); // index 24


	BTN NFC("nfc", "nfc_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(NFC); // index 25
	BTN briefcase("briefcase", "briefcase_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(briefcase); // index 26
	BTN add("add_1", "add_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(add); // index 27
	BTN Textbox("type_text", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Textbox); // index 28
	BTN PinNumberCancel("PinNumberCancel_1", "PinNumberCancel_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(PinNumberCancel); // index 29

	BTN NFC_1("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(NFC_1); // index 30
	BTN NFC_2("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(NFC_2); // index 31
	BTN NFC_3("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(NFC_3); // index 32
	BTN NFC_4("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(NFC_4); // index 33

	BTN Device_1("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Device_1); // index 34
	BTN Device_2("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Device_2); // index 35
	BTN Device_3("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Device_3); // index 36
	BTN Device_4("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Device_4); // index 37


	BTN Language("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Language); // index 38
	BTN Brightness("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Brightness); // index 39
	BTN CreateBackup("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(CreateBackup); // index 40
	BTN EditWallet("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(EditWallet); // index 41
	BTN Security("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Security); // index 42


	BTN English("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
    BUTTONS.push_back(English); // index 43
	BTN Spanish("submenu_button", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
    BUTTONS.push_back(Spanish); // index 44


	BTN BL0("BL_btnBot_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(BL0); // index 45
	BTN BL1("BL_btn_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(BL1); // index 46
	BTN BL2("BL_btn_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(BL2); // index 47
	BTN BL3("BL_btn_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(BL3); // index 48
	BTN BL4("BL_btnTop_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(BL4); // index 49


	BTN Pin("slider_security_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Pin); // index 50
	BTN Password("slider_security_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	BUTTONS.push_back(Password); // index 51

	BUTTONS.push_back(Textbox); // index 52
	
		/************************ Wallet change buttons ************************/

	BUTTONS.at(30).add_text(&montserrat, "language_database->at(4)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(31).add_text(&montserrat, "language_database->at(5)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(32).add_text(&montserrat, "language_database->at(6)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(33).add_text(&montserrat, "language_database->at(6)", 70, 22, RGB666_WHITE_COLOR);

	BUTTONS.at(34).add_text(&montserrat, "language_database->at(4)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(35).add_text(&montserrat, "language_database->at(5)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(36).add_text(&montserrat, "language_database->at(6)", 70, 22, RGB666_WHITE_COLOR);
	BUTTONS.at(37).add_text(&montserrat, "language_database->at(6)", 70, 22, RGB666_WHITE_COLOR);

	/***********************************Keyboard buttons***************************************/

	vector<BTN> Keyboard_BTN;

	//Initialize buttons that are not affected by changing from lower to upper keyboard
	BTN key_0("keyboard_button_number0_1", "keyboard_button_number0_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_0); //index 0
	BTN key_1("keyboard_button_number1_1", "keyboard_button_number1_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_1); //index 1
	BTN key_2("keyboard_button_number2_1", "keyboard_button_number2_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_2); //index 2
	BTN key_3("keyboard_button_number3_1", "keyboard_button_number3_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_3); //index 3
	BTN key_4("keyboard_button_number4_1", "keyboard_button_number4_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_4); //index 4
	BTN key_5("keyboard_button_number5_1", "keyboard_button_number5_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_5); //index 5
	BTN key_6("keyboard_button_number6_1", "keyboard_button_number6_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_6); //index 6
	BTN key_7("keyboard_button_number7_1", "keyboard_button_number7_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_7); //index 7
	BTN key_8("keyboard_button_number8_1", "keyboard_button_number8_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_8); //index 8
	BTN key_9("keyboard_button_number9_1", "keyboard_button_number9_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_9); //index 9

	BTN key_shift("keyboard_shift_1", "keyboard_shift_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_shift); //index 10 
	BTN key_shift_active("keyboard_shift_2", "keyboard_shift_1", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_shift_active); //index 11
	BTN key_erase("keyboard_erase_1", "keyboard_erase_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_erase); //index 12
	BTN key_dot("keyboard_dot_1", "keyboard_dot_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_dot); //index 13
	BTN key_space("keyboard_space_1", "keyboard_space_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_space); //index 14
	BTN key_enter("keyboard_enter_1", "keyboard_enter_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_enter); //index 15

	//Lower case letters
	BTN key_ql("keyboard_button_ql_1", "keyboard_button_ql_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_ql); //index 16
	BTN key_wl("keyboard_button_wl_1", "keyboard_button_wl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_wl); //index 17
	BTN key_el("keyboard_button_el_1", "keyboard_button_el_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_el); //index 18
	BTN key_rl("keyboard_button_rl_1", "keyboard_button_rl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_rl); //index 19
	BTN key_tl("keyboard_button_tl_1", "keyboard_button_tl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_tl); //index 20
	BTN key_yl("keyboard_button_yl_1", "keyboard_button_yl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_yl); //index 21
	BTN key_ul("keyboard_button_ul_1", "keyboard_button_ul_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_ul); //index 22
	BTN key_il("keyboard_button_il_1", "keyboard_button_il_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_il); //index 23
	BTN key_ol("keyboard_button_ol_1", "keyboard_button_ol_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_ol); //index 24
	BTN key_pl("keyboard_button_pl_1", "keyboard_button_pl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_pl); //index 25

	BTN key_al("keyboard_button_al_1", "keyboard_button_al_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_al); //index 26
	BTN key_sl("keyboard_button_sl_1", "keyboard_button_sl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_sl); //index 27
	BTN key_dl("keyboard_button_dl_1", "keyboard_button_dl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_dl); //index 28
	BTN key_fl("keyboard_button_fl_1", "keyboard_button_fl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_fl); //index 29
	BTN key_gl("keyboard_button_gl_1", "keyboard_button_gl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_gl); //index 30
	BTN key_hl("keyboard_button_hl_1", "keyboard_button_hl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_hl); //index 31
	BTN key_jl("keyboard_button_jl_1", "keyboard_button_jl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_jl); //index 32
	BTN key_kl("keyboard_button_kl_1", "keyboard_button_kl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_kl); //index 33
	BTN key_ll("keyboard_button_ll_1", "keyboard_button_ll_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_ll); //index 34

	BTN key_zl("keyboard_button_zl_1", "keyboard_button_zl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_zl); //index 35
	BTN key_xl("keyboard_button_xl_1", "keyboard_button_xl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_xl); //index 36
	BTN key_cl("keyboard_button_cl_1", "keyboard_button_cl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_cl); //index 37
	BTN key_vl("keyboard_button_vl_1", "keyboard_button_vl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_vl); //index 38
	BTN key_bl("keyboard_button_bl_1", "keyboard_button_bl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_bl); //index 39
	BTN key_nl("keyboard_button_nl_1", "keyboard_button_nl_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_nl); //index 40
	BTN key_ml("keyboard_button_ml_1", "keyboard_button_ml_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_ml); //index 41

	//Upper case letters
	BTN key_Q("keyboard_button_Q_1", "keyboard_button_Q_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_Q); //index 42
	BTN key_W("keyboard_button_W_1", "keyboard_button_W_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_W); //index 43
	BTN key_E("keyboard_button_E_1", "keyboard_button_E_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_E); //index 44
	BTN key_R("keyboard_button_R_1", "keyboard_button_R_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_R); //index 45
	BTN key_T("keyboard_button_T_1", "keyboard_button_T_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_T); //index 46
	BTN key_Y("keyboard_button_Y_1", "keyboard_button_Y_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_Y); //index 47
	BTN key_U("keyboard_button_U_1", "keyboard_button_U_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_U); //index 48
	BTN key_I("keyboard_button_I_1", "keyboard_button_I_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_I); //index 49
	BTN key_O("keyboard_button_O_1", "keyboard_button_O_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_O); //index 50
	BTN key_P("keyboard_button_P_1", "keyboard_button_P_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_P); //index 51

	BTN key_A("keyboard_button_A_1", "keyboard_button_A_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_A); //index 52
	BTN key_S("keyboard_button_S_1", "keyboard_button_S_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_S); //index 53
	BTN key_D("keyboard_button_D_1", "keyboard_button_D_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_D); //index 54
	BTN key_F("keyboard_button_F_1", "keyboard_button_F_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_F); //index 55
	BTN key_G("keyboard_button_G_1", "keyboard_button_G_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_G); //index 56
	BTN key_H("keyboard_button_H_1", "keyboard_button_H_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_H); //index 57
	BTN key_J("keyboard_button_J_1", "keyboard_button_J_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_J); //index 58
	BTN key_K("keyboard_button_K_1", "keyboard_button_K_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_K); //index 59
	BTN key_L("keyboard_button_L_1", "keyboard_button_L_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_L); //index 60

	BTN key_Z("keyboard_button_Z_1", "keyboard_button_Z_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_Z); //index 61
	BTN key_X("keyboard_button_X_1", "keyboard_button_X_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_X); //index 62
	BTN key_C("keyboard_button_C_1", "keyboard_button_C_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_C); //index 63
	BTN key_V("keyboard_button_V_1", "keyboard_button_V_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_V); //index 64
	BTN key_B("keyboard_button_B_1", "keyboard_button_B_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_B); //index 65
	BTN key_N("keyboard_button_N_1", "keyboard_button_N_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_N); //index 66
	BTN key_M("keyboard_button_M_1", "keyboard_button_M_2", &hashbankdb, &GLCD, &tscrn_ctrl, 0, 0.0);
	Keyboard_BTN.push_back(key_M); //index 67

	GLCD.lcd_init();
	Background.render_image(0,0);
/*
	// Start thread ---- USB controller
	struct USB_controller ud;
	struct usb_action usbpage;
	ud.thread_id = 2;
	ud.USB_CONTENT = &usbpage;
	ud.IMAGES = &IMAGES;
	ud.montserrat = &montserrat;
	ud.hashbankdb = &hashbankdb;
	ud.LOAD_CTRL = &ld;
	// Threads
	pthread_t listen_usb;
	rc = pthread_create(&listen_usb, NULL, USB_commands, (void *)&ud);
	if (rc) {
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}
*/
	/**********************************Backlight*************************************************/
	    
	string pwm_pin_mux = "echo pwm > /sys/bus/platform/devices/ocp:P1_36_pinmux/state";
	char * PWMMUX = new char[pwm_pin_mux.length()+1];
	strcpy (PWMMUX, pwm_pin_mux.c_str());
	cout << "Pinmuxing PWM " << endl;
	int script_status = system(PWMMUX); // Run script! if returned 0 -- sucess
	delete[] PWMMUX;

	string Percent = hashbankdb.sql_vals[1];
	Percent += "0000"; // PWM entries are in nanoseconds, multiplier.
	PWM Backlight("0", "0"); //LCD connected to channel 0 on port 0.
	Backlight.run("1000000", Percent); // 1kHz signal, Percent duty cycle.
	int BL_value;
	BL_value = stoi(Percent) / 200000;
	cout << "Backlight state is: "<< BL_value << endl;

	hashbankdb.sql_query("SELECT language, brightness, pin, password FROM user_settings where id=1"); // Records returned
	string password = hashbankdb.sql_vals[3];
	bool  pin_enable = false, password_enable = false;
	if (hashbankdb.sql_vals[2] != "") {
		pin_enable = true;
		cout << "hay pin" << endl;
	}
	if (hashbankdb.sql_vals[3] != "") {
		password_enable = true;
		cout << "hay contrasena" << endl;
	}

    hashbankdb.sql_query("SELECT * FROM language where language_id = " + hashbankdb.sql_vals[0]); // Records returned
    vector<string> language_database = hashbankdb.sql_vals;
	int WalletID;
	bool lock = true, password_lock = false, pin_lock = false;
	
	do {
		// --- TEST OF LANDSCAPE MODE!!
		if (lock) {
			if(password_enable){
				password_lock = password_verification(&Keyboard_BTN, &BUTTONS, &IMAGES, &language_database, &montserrat, &hashbankdb, &GLCD, &tscrn_ctrl, &pts, password);
			}else{
				password_lock = true;
			}
			if(pin_enable){
				pin_lock = pin_code_verification(&BUTTONS, &language_database, &montserrat, &hashbankdb, &GLCD, &tscrn_ctrl, &pts, &Background, &pin_line, &Text_eraser_partial, &logo);
			}else{
				pin_lock = true;
			}
		}
		if(password_lock && pin_lock){
			WalletID = loadChangeWallet(&IMAGES, &Keyboard_BTN, &BUTTONS, &language_database, &montserrat, &hashbankdb, &GLCD, &tscrn_ctrl, &pts, &ld);
			lock = loadSidebar(WalletID, &IMAGES, &Keyboard_BTN, &BUTTONS, &language_database, &Backlight, BL_value, &montserrat, &hashbankdb, &GLCD, &tscrn_ctrl, &pts, &ld, &pin_enable, &password_enable);
		}
		cout << "Reiniciando" << endl;
	} while (1);
	
	pthread_exit(NULL);
	

	return 0;
	
}