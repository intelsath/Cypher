#ifndef _BTN_H
#define _BTN_H

#include <string>

#include "touchctrl.h"
#include "stopwatch.h"
#include "GFX.h"
#include "font.h"

#define BTN_UNPRESSED 0 
#define BTN_PRESSED 1

class BTN : public GFX{
	public:
		BTN(std::string, std::string, SQLDB*, ILI9488*, TOUCHCTRL *tscrn_ctrl, bool, double=0.5, bool=false);
		BTN(std::string, SQLDB*, ILI9488*, TOUCHCTRL *tscrn_ctrl, bool, double=0.5, bool=false); // To create buttons with no visual response.
		BTN(BTN*, SQLDB*, ILI9488*, TOUCHCTRL *tscrn_ctrl, bool, double=0.5, bool=false); // Use this when bitmap is already created (recycle same bitmap object instead of loading the same texture again)

		void general_settings(SQLDB*, ILI9488*, TOUCHCTRL*, bool, bool); // Load general settings common to both overloaded constructor

		// Add text to button
		void add_text(FONT*, std::string, unsigned int, unsigned int, uint32_t);

		void init(unsigned int, unsigned int, bool=false ); // This will render button for the first time
        bool event_handler(unsigned int, unsigned int, bool=false); // Returns true if there is a triggered event

		bool is_being_pressed(unsigned int, unsigned int); // Is the button being pressed?

	private:
		bool pressed, trigger; // Should we trigger an event to change in the status of the button?
		unsigned int max_boundx, max_boundy; // Max bounds
		unsigned int min_boundx, min_boundy; // Min bounds

		static const unsigned int percentage_touch = 10; // The accuracy +/-% that it must have 
		static bool button_is_enabled; // Enable or disable buttons -- Static variable
		static STOPWATCH timer; // Timer to decide when to enable buttons again

		double time_touch; // Time between touches

		// Noise elimination -- minimum samples to trigger an event
		bool PRESSED, TRIGGER; // Use this uppercase bools to determine true pressed/trigger changes (smoothed values)

		bool text_exist; // Does this button has text embedded on it?
		std::string btn_text; // Text on button
		unsigned int txtx, txty, txtxmax, txtymax; // Position of text relative to button
		uint32_t text_color; // color of text

		TOUCHCTRL *tscrn_ctrl;
		FONT *textfont; // Add text to button?

		bool init_selected; // Was this button initialized as "selected"?
		bool btn_type; // To get (or not) a visual response.
		

};


#endif