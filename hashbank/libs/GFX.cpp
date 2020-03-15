// GFX class -- Graphics controller -- Every object contains a single bitmap 

#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include "GFX.h"

#include <cassert>

using namespace std;

// Pixel MAP (last rendered frame)
const int BITMAP::lcd_width = GFX::get_lcd_width(); 
const int BITMAP::lcd_height = GFX::get_lcd_height(); 
// Initialize static variables
bool BITMAP::frame_lkptble_initialized = false;
// Pixel Map Positions' look up table (matrix)
vector< vector<BITMAP::PIXEL> > BITMAP::pixel_map;
vector< vector<BITMAP::PIXEL> > BITMAP::pixel_map_landscape;

// default for protrait mode
int GFX::LCD_ORIENTATION = LCD_PORTRAIT; 
int GFX::lcd_height = 480;
int GFX::lcd_width = 320;

BITMAP::BITMAP(vector<uint32_t> BMP1, std::string filename, int width, int height, bool able_to_clear)
{
	BMP = BMP1;
	if( able_to_clear )  // Does this obj has a clear() function?
		clear_pixels_BMP = BMP; 
	this->filename = filename;
	this->width = width;
	this->height = height;

	compute_lkuptable();
}

// Compute lookup table for special pixels (Transparent or colored pixels)
// u32 position and byte_position
void BITMAP::compute_lkuptable()
{
	int buffer32 = BMP.size();
	int bytes = buffer32*4;
	int total_pixels = bytes/GFX::get_bytes_per_pixel(); // <R, G, B> sets, a.k.a total pixels

	int byte_iterator = 0; // Keeps track of byte element
	// int pixel_position;       // pixel's position in current uint32_t 
	// int color_bytepos;        // Pixel's specific color byte position
	// int shiftbits;            // For bitwise operations -- tell how much to shift bits
	// u32 buffer
	// uint32_t *u32buf = new uint32_t[buffer32];

	if( !frame_lkptble_initialized ) // Is it initialized yet? ... Static variables will only get initialized once
	{
		int total_pixels_frame = lcd_width * lcd_height;
		cout << "Creating lookup table for the whole frame..." << endl;

		// Initialize vector pixel_map
		// int rows = GFX::get_lcd_width();
		int rows = GFX::get_lcd_width();
		int cols = GFX::get_lcd_height();
		for( int i=0; i<rows; i++ )
		{
			// It's a vector of vectors
			vector<PIXEL> pixels_cols;
			for( int j=0;j<cols;j++)
			{
				PIXEL pixel_default;
				pixels_cols.push_back(pixel_default);
			}
			this->pixel_map.push_back(pixels_cols);
		}

		// Initialize vector pixel_map
		// int rows = GFX::get_lcd_width();
		rows = GFX::get_lcd_height();
		cols = GFX::get_lcd_width();
		for( int i=0; i<rows; i++ )
		{
			// It's a vector of vectors
			vector<PIXEL> pixels_cols;
			for( int j=0;j<cols;j++)
			{
				PIXEL pixel_default;
				pixels_cols.push_back(pixel_default);
			}
			this->pixel_map_landscape.push_back(pixels_cols);
		}

		// pixel iterators 
		int x0 = 0;
		int y0 = 0;
		int pxitr_x = x0; 
		int pxitr_y = y0; 
		// landscape
		int pxitrls_x = x0; 
		int pxitrls_y = y0; 
		int xn = lcd_width-1; // max size
		int xn_ls = lcd_height-1; // max size
		for(int i=0; i<total_pixels_frame; i++)
		{
			uint32_t mask = 0xFF000000;
		
			// Extract current pixel!
			// Pixels are stored in three-tuples, so get the all the sets of three bytes
			// Find byte position of each pixel
			// RED
			int pixelR_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
			int colorR_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits
			if( colorR_bytepos == -1 && byte_iterator != 0) colorR_bytepos = 3; // Last element   

			// GREEN
			byte_iterator++; 
			int pixelG_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
			int colorG_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits]
			if( colorG_bytepos == -1 && byte_iterator != 0) colorG_bytepos = 3; // Last element

			// BLUE
			byte_iterator++; 
			int pixelB_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
			int colorB_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits
			if( colorB_bytepos == -1 && byte_iterator != 0) colorB_bytepos = 3; // Last element

			byte_iterator++; 

			// Obj of lookup table
			PIXEL all_pixels_lkuptbl;

			// Save look up table
			// Byte positions
			all_pixels_lkuptbl.byte_positionR = colorR_bytepos;
			all_pixels_lkuptbl.byte_positionG = colorG_bytepos;
			all_pixels_lkuptbl.byte_positionB = colorB_bytepos;
			// Pixel Positions
			all_pixels_lkuptbl.u32_positionR = pixelR_position;
			all_pixels_lkuptbl.u32_positionG = pixelG_position;
			all_pixels_lkuptbl.u32_positionB = pixelB_position;

			// Set pixels
			all_pixels_lkuptbl.pos_x = pxitr_x;
			all_pixels_lkuptbl.pos_y = pxitr_y;

			// Compute x,y
			// Save on matrix ["map" of all the screen]
			this->pixel_map[pxitr_x][pxitr_y] = all_pixels_lkuptbl;

			pxitr_x++;
			// Reset values -- Pixels are rendered from X0 to Xn (horizontally)
			if( pxitr_x > xn )
			{
				pxitr_x = x0;
				pxitr_y++;
			}

			// Obj of lookup table
			PIXEL all_pixels_lkuptbl_landscape;

			// Save look up table
			// Byte positions
			all_pixels_lkuptbl_landscape.byte_positionR = colorR_bytepos;
			all_pixels_lkuptbl_landscape.byte_positionG = colorG_bytepos;
			all_pixels_lkuptbl_landscape.byte_positionB = colorB_bytepos;
			// Pixel Positions
			all_pixels_lkuptbl_landscape.u32_positionR = pixelR_position;
			all_pixels_lkuptbl_landscape.u32_positionG = pixelG_position;
			all_pixels_lkuptbl_landscape.u32_positionB = pixelB_position;

			// Set pixels
			all_pixels_lkuptbl_landscape.pos_x = pxitrls_x;
			all_pixels_lkuptbl_landscape.pos_y = pxitrls_y;


			// Landscape pixel map
			this->pixel_map_landscape[pxitrls_x][pxitrls_y] = all_pixels_lkuptbl_landscape;
			
			pxitrls_x++;
			// Reset values -- Pixels are rendered from X0 to Xn (horizontally)
			if( pxitrls_x > xn_ls )
			{
				pxitrls_x = x0;
				pxitrls_y++;
			}

			// done! 
			frame_lkptble_initialized = true; // Frame map initialized!
		}
	}

	byte_iterator = 0;
	cout << "Creating lookup table for this BMP's pixels..." << endl;

	for(int i=0; i<total_pixels; i++)
	{
		uint32_t uint24Tpixel = 0x00000000; // 3-byte Pixel
		uint32_t mask = 0xFF000000;
		
		// Extract current pixel!
		// Pixels are stored in three-tuples, so get the all the sets of three bytes
		// Find byte position of each pixel
		unsigned int pixelR_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
		uint8_t colorR_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits
		if( colorR_bytepos == -1 && byte_iterator != 0) colorR_bytepos = 3; // Last element   
		int shiftbitsR = colorR_bytepos*8;
		uint8_t Rpx = (BMP[pixelR_position]&(mask>>shiftbitsR))>>((3-colorR_bytepos)*8); // RED
		byte_iterator++; 
		unsigned int pixelG_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
		uint8_t colorG_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits]
		if( colorG_bytepos == -1 && byte_iterator != 0) colorG_bytepos = 3; // Last element
		int shiftbitsG = colorG_bytepos*8;
		uint8_t Gpx = (BMP[pixelG_position]&(mask>>shiftbitsG))>>((3-colorG_bytepos)*8); // GREEN
		byte_iterator++; 
		unsigned int pixelB_position = floor((double)byte_iterator/4.0); // 4 because uint32_t has 4 bytes
		uint8_t colorB_bytepos = (byte_iterator % 4); // The remainder is the byte position -- will be used for shifting bits
		if( colorB_bytepos == -1 && byte_iterator != 0) colorB_bytepos = 3; // Last element
		int shiftbitsB = colorB_bytepos*8;
		uint8_t Bpx = (BMP[pixelB_position]&(mask>>shiftbitsB))>>((3-colorB_bytepos)*8); // BLUE
		byte_iterator++; 
		
		// Current Pixel
		// uint24Tpixel |= (Rpx<<16) | (Gpx<<8) | Bpx;

		PIXEL this_pixel;
		
		// Lookup
		// Save relevant data
		// color byte position
		this_pixel.byte_positionR = colorR_bytepos; // Byte position R
		this_pixel.byte_positionG = colorG_bytepos; // Byte position G
		this_pixel.byte_positionB = colorB_bytepos; // Byte position B
		// Pixel position in u32 vector
		this_pixel.u32_positionR = pixelR_position; // Vector position R
		this_pixel.u32_positionG = pixelG_position; // Vector position G
		this_pixel.u32_positionB = pixelB_position; // Vector position B

		// Set pixel values
		this_pixel.Rpx = Rpx;
		this_pixel.Gpx = Gpx;
		this_pixel.Bpx = Bpx;

		// Compute x,y
		// Y
		int iter = i; // Use for loop iterator to determine the current pixel (since we are iterating through all pxs)
        double rows = floor(iter / width);
        int iter_y = (int)rows;
        // X
        double cols = iter - (iter_y*width);
        int iter_x = (int)cols;

		// Set pixels
		this_pixel.pos_x = iter_x;
		this_pixel.pos_y = iter_y;

		this->all_pixels.push_back(this_pixel);

	}

	// Set clear pixels
	this->all_clear_pixels = this->all_pixels;
	
	
	// delete[] u32buf;

}

GFX::GFX()
{
	bmp_initialized = false;
	switch_color = 0;  // By default images won't support color change (only for specific types of images such as fonts)
}

void GFX::set_lcd_orientation(int orientation)
{
	// Change LCD orientation ILI
	GLCD->lcd_setOrientation(orientation);
	// Make sure GFX knows the new LCD orientation
	LCD_ORIENTATION = orientation;

	switch( LCD_ORIENTATION )
	{
		case LCD_PORTRAIT:
			lcd_height = 480;
			lcd_width = 320;
		break;

		case LCD_LANDSCAPE:
			lcd_height = 320;
			lcd_width = 480;
		break;
	}

}

// Get data from sql
void GFX::get_bmp_data(string filename, int *width, int *height, int *totalelems32, int *total_bytes)
{
	// NOTE: aside from the width and height (which could differ), all other properties must remain the same accross BMPs

	// Sql query
	string sqlqry  = "SELECT Height,Width,Transparency,AlphaHexCode,ColorChange,CCHexCode FROM images WHERE Filename = '" + filename + "'";
	sqldb->sql_query(sqlqry);

	// Was the bmp found?
	int sqlsize = sqldb->sql_vals.size();
	if ( sqlsize <= 0 )
		cout << "SQL " << filename << " BMP not found!" << endl;
	assert(sqlsize>0);

	*height = stoi(sqldb->sql_vals[0]); // Height
	*width = stoi(sqldb->sql_vals[1]); // Width
	
	if( !bmp_initialized ) // Do not call this function again once the first bmp has been initialized (in case there are multiple bmps)
	{
		trasnparency = stoi(sqldb->sql_vals[2]); // Transparency
		alphahex = stoi(sqldb->sql_vals[3]); // AlphaHexCode
		switch_color = stoi(sqldb->sql_vals[4]); // Is this bmp able to change color?
		cchexcode = stoi(sqldb->sql_vals[5]); // color change hex code
		bmp_initialized = true;
	}

	int numbytes_store = 4; // Bytes will be stored in 4-tupples (uint32t)
	*total_bytes = ((*height) * (*width)*3); // Actual bytes (x3 because RGB666 requires 3 bytes)
	*totalelems32 = (*total_bytes)/numbytes_store;

	int total32 = (*total_bytes) % numbytes_store;

	if( total32 != 0 )
	{
		cout << "WARNING: Image resolution product must be divisible by four as part of optmization.\n i.e Tip: Use 2^x as one of the dimensions where x >= 2. " << endl;
		assert(total32 == 0);
	}

} 

void GFX::set_bmp(string filename)
{
	int width;
	int height;
	int totalelems32;
	int total_bytes;

	get_bmp_data(filename,&width,&height,&totalelems32,&total_bytes);

	// Filename
	cout << "opening: " << filename << endl;
	str:string dirfilename = "rgb666/" + filename;
	char * cstr_file = new char [dirfilename.length()+1];
	strcpy (cstr_file, dirfilename.c_str()); // Copy string to cstr

	// Read image
	cout << "Initializing " << cstr_file << " texture..." << endl;
	char * buffer = new char[total_bytes]; 
	ifstream bmpfile (cstr_file, ios::in | ios::binary);
	bmpfile.read (buffer, total_bytes);
	bmpfile.close();

	vector<uint32_t> BMP1; 

	// Convert to vector<uint32_t>
	int n = 0;
	for(int i=0; i<totalelems32; i++)
	{
		uint32_t u32buffer = 0x00000000; 
		u32buffer |= (buffer[n]<<24) | (buffer[n+1]<<16) | (buffer[n+2]<<8) | buffer[n+3];

		BMP1.push_back(u32buffer);
		n+=4;
	}

	// Does THIS img obj has a clear() function? 
	bool clear_func;
	if( able_to_clear )
		clear_func = true;
	else
		clear_func = false;

	BITMAP bmp_obj(BMP1,filename,width,height,clear_func); // Declare BMP object

	// initialize lookup table for special pixels (either transparent pixels or pixels that can be changed to another color at runtime)
	// Initialize look up table for speciai pixels
	// bmp_obj.compute_lkuptable(BMP1);

	// Push to vector holders
	DEF_BITMAP_HOLDER.push_back(bmp_obj);
	BITMAP_HOLDER = &DEF_BITMAP_HOLDER; // Reference vector

	cout << "done initializing texture!" << endl;

	delete[] cstr_file;
	delete[] buffer;
}

void GFX::ref_bmp(std::vector<BITMAP>* REFD_BMPS, std::string filename)
{
	int null_ptr1, null_ptr2, null_ptr3, null_ptr4;
	get_bmp_data(filename,&null_ptr1,&null_ptr2,&null_ptr3,&null_ptr4); // width, height and the rest of properties are already initialized in BMP vector -- the rest of the properties must remain the same between BMP objs
	BITMAP_HOLDER = REFD_BMPS;
}

// Render without worrying about specia pixels or leaving a map of the image... 
// Can be used for clearing images or drawing shapes (e.g rectangles with same color)
void GFX::fast_render(unsigned int x, unsigned int y, BITMAP *bmp_render, bool clear_image)
{
	int sizepxs = bmp_render->all_pixels.size();
	// pixel iterator (save it to pixel_map)
	int pxitr_x, pxitr_y;
	pxitr_x = x; 
	pxitr_y = y; 
	// Render from X0,Y0 "TO" Xn,Yn
	unsigned int xn, yn;
	xn = x + bmp_render->get_width() -1;
	yn = y + bmp_render->get_height() -1;

	for( int i=0;i<sizepxs;i++ ) 
	{
		BITMAP::PIXEL render_px;
		if( clear_image ) // Are we clearing pixels? Or rendering a normal image
			render_px = bmp_render->all_clear_pixels[i];
		else
			render_px = bmp_render->all_pixels[i];

		// Save on matrix ["map" of all the screen]
		//bmp_render->pixel_map[pxitr_x][pxitr_y] = render_px;
		if( LCD_ORIENTATION == LCD_PORTRAIT )
			bmp_render->pixel_map[pxitr_x][pxitr_y] = render_px;
		else
			bmp_render->pixel_map_landscape[pxitr_x][pxitr_y] = render_px;
		pxitr_x++;
		
		// Reset values -- Pixels are rendered from X0 to Xn (horizontally)
		if( pxitr_x > xn )
		{
			pxitr_x = x;
			pxitr_y++;
		}
	}
	
	// Render!
	cout << "Fast rendering..." << bmp_render->get_filename() << " with clear_image: " << clear_image << endl;
	if( clear_image )
		GLCD->lcd_bmp(bmp_render->clear_pixels_BMP,x,y,xn,yn);
	else
		GLCD->lcd_bmp(bmp_render->BMP,x,y,xn,yn);
	
	// Render coordinates
	position_x = x; 
	position_y = y; 
}

void GFX::render(unsigned int x, unsigned int y, BITMAP *bmp_render)
{
	int sizepxs = bmp_render->all_pixels.size();
	// pixel iterator (save it to pixel_map)

	int pxitr_x, pxitr_y;
	pxitr_x = x; 
	pxitr_y = y; 

	// Render from X0,Y0 "TO" Xn,Yn
	unsigned int xn, yn;
	xn = x + bmp_render->get_width() -1;
	yn = y + bmp_render->get_height() -1;

	for( int i=0;i<sizepxs;i++ ) 
	{
		BITMAP::PIXEL render_px = bmp_render->all_pixels[i];

		if( able_to_clear ) // Does this IMG have a clear() function?
		{
			// Pixels located in the back of this BMP, makes it easier to clear it when needed
			// Pixel object
			BITMAP::PIXEL backpx;
			if( LCD_ORIENTATION == LCD_PORTRAIT )
				backpx = bmp_render->pixel_map[pxitr_x][pxitr_y];
			else
				backpx = bmp_render->pixel_map_landscape[pxitr_x][pxitr_y];
			
			uint8_t Rbkpx = backpx.Rpx;
			uint8_t Gbkpx = backpx.Gpx;
			uint8_t Bbkpx = backpx.Bpx;

			bmp_render->all_clear_pixels[i] = backpx; // Set clear pixel

			BITMAP::PIXEL bmp_backpx = render_px; // Bmp pixel object
			// bmp_render->BMP u32 position
			unsigned int pixelbkR_position = bmp_backpx.u32_positionR;
			unsigned int pixelbkG_position = bmp_backpx.u32_positionG;
			unsigned int pixelbkB_position = bmp_backpx.u32_positionB;
			uint8_t colorbkR_bytepos = bmp_backpx.byte_positionR;
			uint8_t colorbkG_bytepos = bmp_backpx.byte_positionG;
			uint8_t colorbkB_bytepos = bmp_backpx.byte_positionB;

			// Save clear_pixels map
			uint32_t maskbk = 0x000000FF;
			bmp_render->clear_pixels_BMP[pixelbkR_position] = (bmp_render->clear_pixels_BMP[pixelbkR_position]&~(maskbk<<((3-colorbkR_bytepos)*8))) | Rbkpx<<((3-colorbkR_bytepos)*8);
			bmp_render->clear_pixels_BMP[pixelbkG_position] = (bmp_render->clear_pixels_BMP[pixelbkG_position]&~(maskbk<<((3-colorbkG_bytepos)*8))) | Gbkpx<<((3-colorbkG_bytepos)*8);
			bmp_render->clear_pixels_BMP[pixelbkB_position] = (bmp_render->clear_pixels_BMP[pixelbkB_position]&~(maskbk<<((3-colorbkB_bytepos)*8))) | Bbkpx<<((3-colorbkB_bytepos)*8);
		}

		if( trasnparency )
		{
			uint32_t uint24Tpixel = 0x00000000; // 3-byte Pixel
			uint24Tpixel |= (render_px.Rpx<<16) | (render_px.Gpx<<8) | render_px.Bpx;

			// This pixel is supposed to be transparent!
			if( uint24Tpixel == alphahex )
			{
				// Pixel object
				//BITMAP::PIXEL specialpx = bmp_render->pixel_map[pxitr_x][pxitr_y];
				BITMAP::PIXEL specialpx;
				if( LCD_ORIENTATION == LCD_PORTRAIT )
					specialpx = bmp_render->pixel_map[pxitr_x][pxitr_y];
				else
					specialpx = bmp_render->pixel_map_landscape[pxitr_x][pxitr_y];

				uint8_t Rpx = specialpx.Rpx;
				uint8_t Gpx = specialpx.Gpx;
				uint8_t Bpx = specialpx.Bpx;

				BITMAP::PIXEL bmp_specialpx = render_px; // Bmp pixel object
				// bmp_render->BMP u32 position
				unsigned int pixelR_position = bmp_specialpx.u32_positionR;
				unsigned int pixelG_position = bmp_specialpx.u32_positionG;
				unsigned int pixelB_position = bmp_specialpx.u32_positionB;
				uint8_t colorR_bytepos = bmp_specialpx.byte_positionR;
				uint8_t colorG_bytepos = bmp_specialpx.byte_positionG;
				uint8_t colorB_bytepos = bmp_specialpx.byte_positionB;

				// Replace pixels!
				uint32_t mask2 = 0x000000FF;
				bmp_render->BMP[pixelR_position] = (bmp_render->BMP[pixelR_position]&~(mask2<<((3-colorR_bytepos)*8))) | Rpx<<((3-colorR_bytepos)*8);
				bmp_render->BMP[pixelG_position] = (bmp_render->BMP[pixelG_position]&~(mask2<<((3-colorG_bytepos)*8))) | Gpx<<((3-colorG_bytepos)*8);
				bmp_render->BMP[pixelB_position] = (bmp_render->BMP[pixelB_position]&~(mask2<<((3-colorB_bytepos)*8))) | Bpx<<((3-colorB_bytepos)*8);
				
				// Pixel to be rendered
				render_px = specialpx;
			}
			// bmp_render->frame_lkuptble[][];
		}

		if( switch_color )
		{
			uint32_t uint24Tpixel = 0x00000000; // 3-byte Pixel
			uint24Tpixel |= (render_px.Rpx<<16) | (render_px.Gpx<<8) | render_px.Bpx;

			// cout << "uint24Tpixel " << uint24Tpixel << endl;
			// cout << "alphahex " << alphahex << endl;
			// This pixel is supposed to be transparent!
			if( uint24Tpixel == cchexcode )
			{
				//BITMAP::PIXEL new_coloredpx = bmp_render->pixel_map[pxitr_x][pxitr_y];
				BITMAP::PIXEL new_coloredpx;
				if( LCD_ORIENTATION == LCD_PORTRAIT )
					new_coloredpx = bmp_render->pixel_map[pxitr_x][pxitr_y];
				else
					new_coloredpx = bmp_render->pixel_map_landscape[pxitr_x][pxitr_y];

				uint32_t mask_cc = 0x000000FF;
				new_coloredpx.Rpx = (new_color&(mask_cc<<16))>>16;
				new_coloredpx.Gpx = (new_color&(mask_cc<<8))>>8;
				new_coloredpx.Bpx = new_color&mask_cc;

				uint8_t Rpx = new_coloredpx.Rpx;
				uint8_t Gpx = new_coloredpx.Gpx;
				uint8_t Bpx = new_coloredpx.Bpx;

				BITMAP::PIXEL bmp_specialpx = render_px; // Bmp pixel object
				// bmp_render->BMP u32 position
				unsigned int pixelR_position = bmp_specialpx.u32_positionR;
				unsigned int pixelG_position = bmp_specialpx.u32_positionG;
				unsigned int pixelB_position = bmp_specialpx.u32_positionB;
				uint8_t colorR_bytepos = bmp_specialpx.byte_positionR;
				uint8_t colorG_bytepos = bmp_specialpx.byte_positionG;
				uint8_t colorB_bytepos = bmp_specialpx.byte_positionB;

				// Replace pixels!
				uint32_t mask2 = 0x000000FF;
				bmp_render->BMP[pixelR_position] = (bmp_render->BMP[pixelR_position]&~(mask2<<((3-colorR_bytepos)*8))) | Rpx<<((3-colorR_bytepos)*8);
				bmp_render->BMP[pixelG_position] = (bmp_render->BMP[pixelG_position]&~(mask2<<((3-colorG_bytepos)*8))) | Gpx<<((3-colorG_bytepos)*8);
				bmp_render->BMP[pixelB_position] = (bmp_render->BMP[pixelB_position]&~(mask2<<((3-colorB_bytepos)*8))) | Bpx<<((3-colorB_bytepos)*8);

				// Pixel to be rendered
				render_px = new_coloredpx;

			}
			// bmp_render->frame_lkuptble[][];
		}

		// Save on matrix ["map" of all the screen]
		//bmp_render->pixel_map[pxitr_x][pxitr_y] = render_px;
		if( LCD_ORIENTATION == LCD_PORTRAIT )
			bmp_render->pixel_map[pxitr_x][pxitr_y] = render_px;
		else
			bmp_render->pixel_map_landscape[pxitr_x][pxitr_y] = render_px;
		pxitr_x++;
		
		// Reset values -- Pixels are rendered from X0 to Xn (horizontally)
		if( pxitr_x > xn )
		{
			pxitr_x = x;
			pxitr_y++;
		}


	}

	//cout << "Rendering..." << bmp_render->get_filename() << endl;
	GLCD->lcd_bmp(bmp_render->BMP,x,y,xn,yn);

	position_x = x; 
	position_y = y; 

}
