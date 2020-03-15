// Button class -- Inherited from GFX, image object
// Button with behavior when being touched

#include <iostream>
#include "BTN.h"

// Disable button when another button was just pressed -- give screen some time to refresh
bool BTN::button_is_enabled = true;

STOPWATCH BTN::timer; // Static timer

BTN::BTN(std::string filename0, std::string filename1,SQLDB* sqldb, ILI9488* GLCD, TOUCHCTRL *tscrn_ctrl, bool rotate_prop, double time_touch, bool able_to_clear)
{
	// Define button type
	btn_type = true; // Visual response
	// Time between touches
	this->time_touch = time_touch;
	
	// Init general settings
	general_settings(sqldb, GLCD, tscrn_ctrl, rotate_prop, able_to_clear);

	// Set bitmaps (when being pressed and not being pressed)
	set_bmp(filename0); // Position 0 -- Not pressed 0 
	set_bmp(filename1); // Position 1 -- Pressed 1
}

// Overloaded constructor -- Does not render a second image on press.
BTN::BTN(std::string filename0,SQLDB* sqldb, ILI9488* GLCD, TOUCHCTRL *tscrn_ctrl, bool rotate_prop, double time_touch, bool able_to_clear)
{
	// Define button type
	btn_type = false; // No visual response
	// Time between touches
	this->time_touch = time_touch;
	
	// Init general settings
	general_settings(sqldb, GLCD, tscrn_ctrl, rotate_prop, able_to_clear);

	// Set bitmaps (when being pressed and not being pressed)
	set_bmp(filename0); // Position 0 -- Not pressed 0 
}

// Overloaded constructor -- Do not initialize sprints again, but rather, point to existing BMP
BTN::BTN(BTN* btn,SQLDB* sqldb, ILI9488* GLCD, TOUCHCTRL *tscrn_ctrl, bool rotate_prop, double time_touch, bool able_to_clear)
{
	std::vector<BITMAP> *vect_ptr = btn->BITMAP_HOLDER;
	// Time between touches
	this->time_touch = time_touch;
	// Init general settings
	general_settings(sqldb, GLCD, tscrn_ctrl, rotate_prop, able_to_clear);

	// Initialize BITMAP vector 
	ref_bmp(vect_ptr, (*vect_ptr)[0].get_filename()); // Deference vector and get first element to get first bitmap's settings (every bmp should have the same settings)

}

// Add text to button
void BTN::add_text(FONT* textfont, std::string btn_text, unsigned int txtx, unsigned int txty, uint32_t text_color)
{
	this->text_exist = true;                // Confirm that this button has text
	this->textfont = textfont;              // Font object
	this->text_color = text_color;          // Text color
	this->btn_text = btn_text;              // Text on this button
	this->txtx = txtx;	        // Position of text in x relative to button
	this->txty = txty;           // Position of text in y relative to button
	this->txtxmax = (*BITMAP_HOLDER)[0].get_width(); // Boundaries
	this->txtymax = (*BITMAP_HOLDER)[0].get_height(); // Boundaries

	// Render text for the first time
	this->textfont->set_text(this->btn_text, TEXT_LINE_MODE);
	//this->textfont->render_text(this->txtx,this->txty,this->text_color,this->txtxmax,this->txtymax);
}

void BTN::general_settings(SQLDB* sqldb, ILI9488* GLCD, TOUCHCTRL *tscrn_ctrl, bool rotate_prop, bool able_to_clear)
{
	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Touch controller
	this->tscrn_ctrl = tscrn_ctrl;
	// Should this image have a clear() function? -- Note: it takes more RAM and processing power, but improves efficiency in small images
	this->able_to_clear = able_to_clear;

	pressed = false;
	trigger = false;
	// Noise reduction variables
	PRESSED = false; 
	TRIGGER = false;

	text_exist = false;
}

bool BTN::is_being_pressed(unsigned int pt_x, unsigned int pt_y)
{
	// Compute coordinates and determine if it's being pressed
	// tscrn_ctrl->compute_coordinates();

	bool x_range = false; // Cursor within the x range 
	bool y_range = false; // Cursor within the y range 
	if( pt_x>= min_boundx && pt_x <= max_boundx )
		x_range = true;
	if( pt_y >= min_boundy && pt_y <= max_boundy )
		y_range = true;


	if( x_range && y_range ) 
	{
		PRESSED = true;
	}
	else{
		PRESSED = false;
	}

	/*if( x_range && y_range ) // Pressed
	{
		// New change?
		if (!pressed)
		{
			trigger = true;
			// current_sample = 0;
			timer.reset();
		}
			
		else
		{
			trigger = false;
			// current_sample++;
			timer.start();
		}
			
		pressed = true;
	}
	else
	{
		// New change?
		if (pressed)
		{
			trigger = true;
			// current_sample = 0;
			timer.reset();
		}
		else
		{
			trigger = false;
			// current_sample++;
			timer.start();
		}
			
		pressed = false;
	}
		
	// A real event was detected 
	//if( current_sample >= sampling_num )
	if( timer.get_epoch() >= time_touch )
	{
		// New change?
		if (PRESSED != pressed)
			TRIGGER = true;
		else
			TRIGGER = false;	
		PRESSED = pressed;
	}*/

	return PRESSED;

}


void BTN::init(unsigned int x, unsigned int y, bool selected)
{

	// Render image
	if( !selected )
		render(x,y,&(*BITMAP_HOLDER)[BTN_UNPRESSED]);
	else 
	{
		if(btn_type)
			render(x,y,&(*BITMAP_HOLDER)[BTN_PRESSED]);
	}

	// Button was initialized as selected
	init_selected = selected;

	// Render text (if any)
	//std::cout << "text_exist: " << text_exist << std::endl;
	//std::cout << "this->txtxmax: " << this->txtxmax << std::endl;
	if( text_exist )
		textfont->render_text(txtx + position_x,txty + position_y,text_color,position_x+txtxmax,position_y+txtymax);

	// Bounds offset (to give it more room for pressing buttons)
	// Init bounds
	min_boundx = position_x;
	if( min_boundx >= percentage_touch ) // Make sure there is not a negative value
		min_boundx-=percentage_touch;
	min_boundy = position_y;
	if( min_boundy >= percentage_touch ) // Make sure there is not a negative value
		min_boundy-=percentage_touch;
	// Bounds
	max_boundx = x + (*BITMAP_HOLDER)[BTN_UNPRESSED].get_width();
	if( max_boundx+percentage_touch < get_lcd_height() ) // Make sure there is not out of bounds
		max_boundx+=percentage_touch;
	max_boundy = y + (*BITMAP_HOLDER)[BTN_UNPRESSED].get_height();
	if( max_boundy+percentage_touch < get_lcd_width() ) // Make sure there is not out of bounds
		max_boundy+=percentage_touch;


}

bool BTN::event_handler(unsigned int pt_x, unsigned int pt_y, bool selected)
{
	// Do nothing, button is "pressed"
	if( selected )
		return false;

	bool status = false;
	//std::cout << "timer.get_epoch(): " << timer.get_epoch() << std::endl;
	// Button is disabled
	if( !button_is_enabled )
	{
		// Can buttons be enabled again?
		if( timer.get_epoch() >= time_touch )
		{
			button_is_enabled = true;
		}
		else
		{
			return status; // false
		}
	}
		

	is_being_pressed(pt_x,pt_y);
	// Was this button initialized as selected?
	if( init_selected )
	{
		TRIGGER = true; // Make this a trigger event
		init_selected = false; // Don't go through here again
	}
		
	// New action detected; Trigger event
	//if( TRIGGER )
	//{
	if( PRESSED )
	{
		// Render image of button pressed
		if( !TRIGGER )
		{
			if(btn_type)
			{
				render(position_x,position_y,&(*BITMAP_HOLDER)[BTN_PRESSED]);
				// Render text (if any)
				if( text_exist )
					textfont->render_text(txtx + position_x,txty + position_y,text_color,position_x+txtxmax,position_y+txtymax);
			}
			TRIGGER = true; // Reset
			status = true;

		}
	}
	else
	{
		// Not being pressed (default image)
		if( TRIGGER ){
			if(btn_type)
			{
				render(position_x,position_y,&(*BITMAP_HOLDER)[BTN_UNPRESSED]);
				// Render text (if any)
				if( text_exist )
					textfont->render_text(txtx + position_x,txty + position_y,text_color,position_x+txtxmax,position_y+txtymax);
			}
			TRIGGER = false; // Reset
			button_is_enabled = false; // Disable button temporary
			timer.reset(); // Resets timer
			timer.start();// Start timer to know if button can be enabled again
		}
				
	}
	//}
	

	return status;

}