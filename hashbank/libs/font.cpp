// Image class -- Inherited from GFX, image object
// This is just a simple image with no behavior frem events

#include <iostream>
#include "font.h"

using namespace std;



const std::vector<std::string> FONT::fontchars({"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z",".",",","0","1","2","3","4","5","6","7","8","9","<",">","*","%"}); 
const std::vector<std::string> FONT::font_filenames({"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","al","bl","cl","dl","el","fl","gl","hl","il","jl","kl","ll","ml","nl","ol","pl","ql","rl","sl","tl","ul","vl","wl","xl","yl","zl","DOT","COMMA","0","1","2","3","4","5","6","7","8","9", "lessthan", "greaterthan", "asterisk", "percent"});

FONT::FONT(std::string fontname, SQLDB* sqldb, ILI9488* GLCD, bool rotate_prop, bool able_to_clear)
{
	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Color switch
	this->switch_color = true; // Fonts are able to change colors
	// Should this image have a clear() function? -- Note: it takes more RAM and processing power, but improves efficiency in small images
	this->able_to_clear = able_to_clear;
	// This is a new object
	this->ptr_init = false;

	// Initialize all characters
	for( int i=0; i<font_filenames.size(); i++ )
	{
		string filename = "font_" + fontname + "_" + font_filenames[i];
		set_bmp(filename);

		// Map
		font_map[fontchars[i]] = i; // Initialize map
	}
	
	this->rendered_text = "";
	this->all_text = "";

}

// Overloaded constructor -- Do not initialize sprints again, but rather, point to existing BMP
FONT::FONT(FONT* refd_font, SQLDB* sqldb, ILI9488* GLCD, bool rotate_prop, bool able_to_clear)
{

	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Color switch
	this->switch_color = true; // Fonts are able to change colors
	// Should this image have a clear() function? -- Note: it takes more RAM and processing power, but improves efficiency in small images
	this->able_to_clear = able_to_clear;
	// Set font map
	this->font_map = refd_font->font_map;

	// Object initialized through a pointer to another object
	this->ptr_init = true;

	// Set bitmap
	std::vector<BITMAP> *vect_ptr = refd_font->BITMAP_HOLDER;
	// Initialize BITMAP vector 
	ref_bmp(vect_ptr, (*vect_ptr)[0].get_filename()); // Deference vector and get first element to get  bitmap's settings 
}

void FONT::set_text(std::string all_text, int print_mode)
{
	this->all_text = all_text;
	this->text_to_render = all_text;
	this->print_mode = print_mode;

	this->string_width = 0;

	for( int i=0; i < all_text.size(); i++ ){
		string character( 1, all_text[i] ); // Character initializer (to string) -- constructor of basic string : basic_string(size_type n, charT c, const Allocator& a = Allocator());
		this->string_width += (*BITMAP_HOLDER)[font_map[character]].get_width();
	}

}

void FONT::clear_text()
{
	while( del_text() )
	{
		// Wait until everything is deleted!
	}
}

int FONT::del_text()
{
	if( print_mode == TEXT_LINE_MODE && !ptr_init)
	{
		
		// Size of string
		size_t strsize = rendered_text.size();
		//cout << "strsize: " << strsize << endl;
		if( strsize == 0)
			return 0;

		// Last rendered character (this is the one we are going to delete)
		string last_char( 1, rendered_text[strsize-1] );     
		// Update rendered_text string
		rendered_text = rendered_text.substr(0, strsize-1);

		// New size of string
		strsize = rendered_text.size();
		// Current last rendered character 
		string current_char( 1, rendered_text[strsize-1] );     

		cout << "rendered_text: " << rendered_text << endl;
		cout << "last_char: " << last_char << endl;
		cout << "current_char: " << current_char << endl;

		// this_image -> current character, not the one being erased
		int this_image_width; // Width of current char
		if( current_char != " " ) // Is this is a space? 
		{
			this_image_width = (*BITMAP_HOLDER)[font_map[current_char]].get_width();
		}
		else // Is this a space? It must be same width as "A"
		{
			this_image_width = (*BITMAP_HOLDER)[font_map["A"]].get_width(); // Make spaces the same width as the BMP of "A"
		}

		cout << "current this_image_width: " << this_image_width << endl;

		cout << "position_x: " << position_x << endl;
		cout << "position_y: " << position_y << endl;
		
		// Render image
		if( last_char != " " )
			fast_render(position_x,position_y,&(*BITMAP_HOLDER)[font_map[last_char]],true); // Clear image

		// Update position
		if( strsize > 0)
			position_x-=this_image_width;

	}
	else
	{
		cout << "Warning: deleting text only permitted on line mode AND when it's a new object (i.e not initialized through another font's object)" << endl;
	}

	return 1;
}

void FONT::render_text(unsigned int x0, unsigned int y0, uint32_t textcolor, int xn, int yn, bool append_text)
{
	
	// Text to render
	string text = this->text_to_render;

	//cout << "rendering text..." <<  text << endl;

	// Text color 
	new_color = textcolor; 

	// We are appending text
	size_t current_cursor = 0;
	unsigned int posx;
	unsigned int posy;
	// Are we appending text? Don't render everything, just the new characters
	if( append_text )
	{
		size_t strsize = this->rendered_text.size();
		if( strsize > 0 ) 
		{
			current_cursor = strsize; // Current text rendered size

			// Last rendered character (this is the one we are going to delete)
			string last_char( 1, rendered_text[strsize-1] );    
			int this_image_width = (*BITMAP_HOLDER)[font_map[last_char]].get_width();
			posx = position_x + this_image_width;
			posy = position_y;
		}
		else
		{
			posx = x0;
			posy = y0;
		}

	}
	else
	{
		posx = x0;
		posy = y0;
	}

	string rendered_text;
	// Render each text character
	for( int i=current_cursor; i < text.size(); i++ )  
	{
		//cout << "entering for..." <<  text << endl;
		string character( 1, text[i] ); // Character initializer (to string) -- constructor of basic string : basic_string(size_type n, charT c, const Allocator& a = Allocator());
		int this_image_width = (*BITMAP_HOLDER)[font_map[character]].get_width();
		int this_image_height = (*BITMAP_HOLDER)[font_map[character]].get_height();

		//cout << "this_image_width: " << this_image_width << endl;
		//cout << "posx: " << posx << endl;
		// Print mode: wall or line mode
		if( print_mode == TEXT_WALL_MODE )
		{
			// Are we within boundaries?
			if (posx >= xn-this_image_width-1)
			{
				posx = x0;
				if (posy >= yn-this_image_height-1)
					break; // End of y-axis
				posy += this_image_height;
			}
		}
		else{
			// Are we within boundaries?
			if (posx >= xn-this_image_width-1)
				break;
		}

		if( character != " " ) // Is this is a space? Don't render to screen, just increase iterator
			render(posx,posy,&(*BITMAP_HOLDER)[font_map[character]]); 
		else
		{
			this_image_width = (*BITMAP_HOLDER)[font_map["A"]].get_width(); // Make spaces the same width as the BMP of "A"
			position_x = posx; // Update 'cursor' manually
			position_y = posy; // Update 'cursor' manually
		}
				
		// Save rendered text
		rendered_text.append(character);
		// posx += width; // Move character "cursor"
		posx += this_image_width;		

		//cout << "exiting for..." <<  text << endl;
	}

	if( append_text )
	{
		this->rendered_text += rendered_text;
	}
	else
	{
		this->rendered_text = rendered_text;
	}
	
	//cout << "rendered_text: " << rendered_text << endl;
}

void FONT::move_text(int direction)
{
	if( print_mode == TEXT_LINE_MODE )
	{
		if( direction > 0 )
		{
			string sbstr = all_text; // sub str 
			// Move string to the right
			size_t strsize = sbstr.size();  
			if( direction <= strsize )
			{
				sbstr = sbstr.substr(direction, strsize);  

				cout << "sbstr: " << sbstr << endl;

				// Clear whole string
				clear_text();

				cout << "position_x: " << position_x << endl;
				cout << "position_y: " << position_y << endl;
				cout << "new_color: " << new_color << endl;

				this->text_to_render = sbstr;
				// Update moved text to screen
				render_text(position_x,position_y,new_color);  
			}

		}

		if( direction < 0 )
		{
		
		}
	}
	else{
	
		cout << "Warning: move_text function only permitted on line mode." << endl;
	}
}
