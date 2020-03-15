// Image class -- Inherited from GFX, image object
// This is just a simple image with no behavior frem events

#include <iostream>
#include "IMG.h"

IMG::IMG(std::string filename, SQLDB* sqldb, ILI9488* GLCD, bool rotate_prop, bool able_to_clear)
{

	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Should this image have a clear() function? -- Note: it takes more RAM and processing power, but improves efficiency in small images
	this->able_to_clear = able_to_clear;

	// Set bitmap
	set_bmp(filename);
}

// Overloaded constructor -- Do not initialize sprints again, but rather, point to existing BMP
IMG::IMG(IMG* refd_img, SQLDB* sqldb, ILI9488* GLCD, bool rotate_prop, bool able_to_clear)
{

	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Should this image have a clear() function? -- Note: it takes more RAM and processing power, but improves efficiency in small images
	this->able_to_clear = able_to_clear;

	// Set bitmap
	std::vector<BITMAP> *vect_ptr = refd_img->BITMAP_HOLDER;
	// Initialize BITMAP vector 
	ref_bmp(vect_ptr, (*vect_ptr)[0].get_filename()); // Deference vector and get first element to get first bitmap's settings (every bmp should have the same settings)
}

void IMG::clear_image()
{
	// Render image
	fast_render(position_x,position_y,&(*BITMAP_HOLDER)[0],true); // Clear image
}

void IMG::render_image(unsigned int x, unsigned int y)
{
	position_x = x; 
	position_y = y;

	// Render image
	render(x,y,&(*BITMAP_HOLDER)[0]);
}

