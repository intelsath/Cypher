// Graphics library that directly interfaces with the ILI9488 driver
#ifndef _GFX_H
#define _GFX_H

// Commomn Colors
#define RGB666_BLACK_COLOR			0x00000000
#define RGB666_WHITE_COLOR			0x00FCFCFC 
#define RGB666_RED_COLOR			0x00FC0000 
#define RGB666_GREEN_COLOR			0x0000FC00
#define RGB666_BLUE_COLOR			0x000000FC 
#define RGB666_YELLOW_COLOR			0x00F8FC00 
#define RGB666_PINK_COLOR			0x00FC00FC
#define RGB666_LIGHTGREEN_COLOR		0x0000FC64
#define RGB666_ORANGE_COLOR			0x00FC7C00
// Logo colors
#define RGB666_LIGHTBLUE_COLORLG	0x0028C4E8
#define RGB666_DARKBLUE_COLORLG		0x001484EC
#define RGB666_GRAY_COLORLG			0x003C444C

// LCD Portrait or Landscape
#define LCD_PORTRAIT  0     // See ILI9488::lcd_setOrientation function
#define LCD_LANDSCAPE 1     // See ILI9488::lcd_setOrientation function

#include <vector>
#include <string>
#include <memory>
#include "sqldb.h"
#include "ILI9488.h"

class BITMAP{
	public:
		// Nested Class
		// Special Pixels Look Up table -- To keep track where exactly are the Transparent and colored pixels
		class PIXEL
		{
			friend class BITMAP; // make private members of this nested class accesible to BITMAP
			friend class GFX; // make private members of this nested class accesible to GFX
			public:
				// Default constructor
				PIXEL()
				{
					this->u32_positionR = 0;
					this->u32_positionG = 0;
					this->u32_positionB = 0;
					this->byte_positionR = 0;
					this->byte_positionG = 0;
					this->byte_positionB = 0;
					this->pos_x = 0;
					this->pos_y = 0;
					this->Rpx = 0;
					this->Gpx = 0;
					this->Bpx = 0;
				}
			    //Copy constructor
			    PIXEL(const PIXEL &t) 
				{
					this->u32_positionR = t.u32_positionR;
					this->u32_positionG = t.u32_positionG;
					this->u32_positionB = t.u32_positionB;
					this->byte_positionR = t.byte_positionR;
					this->byte_positionG = t.byte_positionG;
					this->byte_positionB = t.byte_positionB;
					this->pos_x = t.pos_x;
					this->pos_y = t.pos_y;
					this->Rpx = t.Rpx;
					this->Gpx = t.Gpx;
					this->Bpx = t.Bpx;
				}
			    //Assignment operator
				PIXEL& operator=(const PIXEL &t)
				{
					this->u32_positionR = t.u32_positionR;
					this->u32_positionG = t.u32_positionG;
					this->u32_positionB = t.u32_positionB;
					this->byte_positionR = t.byte_positionR;
					this->byte_positionG = t.byte_positionG;
					this->byte_positionB = t.byte_positionB;
					this->pos_x = t.pos_x;
					this->pos_y = t.pos_y;
					this->Rpx = t.Rpx;
					this->Gpx = t.Gpx;
					this->Bpx = t.Bpx;
				} 
			private:
				// Compute look-up table from object (e.g BITMAP)
				// u32 vector position for each color
				unsigned int u32_positionR;  // RED
				unsigned int u32_positionG;  // GREEN
				unsigned int u32_positionB;  // BLUE
				// byte position for each color (for each pixel)
				uint8_t byte_positionR; // RED
				uint8_t byte_positionG; // GREEN
				uint8_t byte_positionB; // BLUE
				// What's the special's pixels position with respect to the origin (0,0)?
				int pos_x; 
				int pos_y;

				// R G B
				uint8_t Rpx;
				uint8_t Gpx;
				uint8_t Bpx;				
		
		};

		std::vector<uint32_t> BMP;
		std::vector<uint32_t> clear_pixels_BMP; // BMP to clear BMP
		BITMAP(std::vector<uint32_t>, std::string, int, int, bool=true);

		std::string get_filename(){return filename;}

		// Lookup table for "special" pixels 
		void compute_lkuptable(); // Compute the exact u32 position and the exact byte position 

		std::vector<PIXEL> all_clear_pixels; // Table with all the info of the pixels in the back (useful when clearing a BMP)
		std::vector<PIXEL> all_pixels; // Table with all pixels' info
		// Lookup table for all rendered pixels
		static std::vector< std::vector<PIXEL> > pixel_map; // All pixels map! vector matrix -- portrait
		static std::vector< std::vector<PIXEL> > pixel_map_landscape; // All pixels map! vector matrix -- landscape

		// Get this specific BMP's width and height
		unsigned int get_width(){return width;}
		unsigned int get_height(){return height;}

	private:
		std::string filename;
		
		static const int lcd_width, lcd_height; // LCD resolution
		unsigned int height, width; // BMP resolution
		static bool frame_lkptble_initialized; // Has the frame's lookup table been initialized yet?
};

class GFX{
	public:
		GFX();
		void set_bmp(std::string);
		void get_bmp_data(std::string, int*, int*, int*, int*);
		void global_bmprop(unsigned int, unsigned int, unsigned int, unsigned int); // Set global bitmap properties
		// Reference vector (to an external bitmap --intended to recycle bmps)
		void ref_bmp(std::vector<BITMAP>*, std::string);

		void render(unsigned int,unsigned int,BITMAP*); // Render image
		// fast_render: Render image without saving clear_pixels_BMP, or looking for special pixels 
		//... Useful when clearing a BMP or rendering an image where all pixels are just a single color 
		void fast_render(unsigned int,unsigned int,BITMAP*,bool); // Last bool is to determine if we are clearing the BMP or rendering something else
		// Portrait or Landscape
		void set_lcd_orientation(int);

		// Max LCD x and y
		static int get_lcd_height(){return lcd_height;}
		static int get_lcd_width(){return lcd_width;}
		static int get_bytes_per_pixel(){return bytes_per_pixel;}
		static int get_bytes_per_sample(){return bytes_per_sample;}

		bool get_able_to_clear(){return able_to_clear;}

	protected:
		SQLDB *sqldb;
		ILI9488 *GLCD; // ILI9488 LCD driver
		 // LCD X and Y resolutions
		static int lcd_height;
		static int lcd_width;
		static const int bytes_per_pixel = 3; // How many bytes per pixel (for RGB666 it's 3 bytes per pixel)
		static const int bytes_per_sample = 4; // How many bytes per sample (for uin32_t it's 4 bytes per SPI word/sample)
		unsigned int trasnparency; uint32_t alphahex; // trasnparency settings
		unsigned int position_x, position_y; // Actual render position of this graphics

		int totalelems32, total_bytes; // total elements comprising of 32 bits, total bytes
		bool rotate_prop; // Does this picture have rotating properties? [Two different ]
		bool bmp_initialized; // Was bmp data initialized already? Don't call sql again

		// Is this graphics able to change colors?
		unsigned int switch_color; uint32_t cchexcode;
		uint32_t new_color; // color change settings

		// Default images holder 
		std::vector<BITMAP> DEF_BITMAP_HOLDER; 
		std::vector<BITMAP> *BITMAP_HOLDER;  // Are we going to use the default holder? Or referernce another one that's already initialized?

		// Does this GFX obj (a.k.a set of bmps) requires to have a clear_pixels_BMP? (Are we able to clear it?) 
		bool able_to_clear;
		// Current orientation of LCD
		static int LCD_ORIENTATION;
};


#endif