// Image class -- Inherited from GFX, image object
// This is just a simple image with no behavior frem events

#include <iostream>
#include "animation.h"

ANIMATION::ANIMATION(std::string animationfile, SQLDB* sqldb, ILI9488* GLCD, bool rotate_prop, unsigned int spritesnum, unsigned int fps)
{

	// Does this image rotates when rotating device?
	this->rotate_prop = rotate_prop;
	// Set sql 
	this->sqldb = sqldb;
	// Set ILI9488 driver
	this->GLCD = GLCD;
	// Frames per second
	this->fps = fps;
	this->secs = 1/(double)fps;

	// Set bitmaps
	for( int i=1; i<=spritesnum; i++ )
	{
		std::string filename = animationfile + std::to_string(i); // Initialize each sprite
		set_bmp(filename); // Set BMPs
	}

	initialized = false; // Not initialized	
}

void ANIMATION::init(unsigned int x, unsigned int y)
{
	// Render image
	render(x,y,&(*BITMAP_HOLDER)[0]);
	// Start timer
	timer.start(); 

	// pos
	position_x = x; 
	position_y = y;

	initialized = true; // Animation initialized
}

void ANIMATION::animate(unsigned int x, unsigned int y)
{
	if( !initialized )
	{
		init(x,y);
	}

	if( timer.get_epoch() >= secs ) 
	{
		// updates new frame
		frame_itr++;
		if( frame_itr >= (*BITMAP_HOLDER).size()  )
			frame_itr = 0;

		// Render image
		render(x,y,&(*BITMAP_HOLDER)[frame_itr]);

		// Reset timer
		timer.reset();
	}

}

