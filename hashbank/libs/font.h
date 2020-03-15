#ifndef _FONT_H
#define _FONT_H

#include <map>
#include "GFX.h"

// Print mode: How to draw text on screen
#define TEXT_WALL_MODE 0 // Wall of text
#define TEXT_LINE_MODE 1 // All in just one line


class FONT : public GFX{
	public:
		FONT(std::string, SQLDB*, ILI9488*,bool,bool=true);
		FONT(FONT*, SQLDB*, ILI9488*,bool,bool=true);

		// Text manipulation fucntions (e.g Clear and delete)
		void clear_text();
		int del_text();
		void move_text(int);
		
		void set_text(std::string, int); // Set text!
		int get_width_pixels(){ return string_width; }
		void render_text(unsigned int, unsigned int, uint32_t, int=GFX::get_lcd_width(), int=GFX::get_lcd_height(),bool=false);
		
	private:
		static const std::vector<std::string> fontchars; 
		static const std::vector<std::string> font_filenames;
		std::map<std::string,int> font_map;

		// Original position
		unsigned int text_position_x0, text_position_y0; // First position_x, position_y(origin of string)

		// Wall or line mode?
		int print_mode;
		bool ptr_init; // Was this object initialized through a pointer?

		std::string all_text; // All the text
		std::string rendered_text; // All visible text on screen
		std::string text_to_render;
		int string_width; // String width in pixels
		
};


#endif