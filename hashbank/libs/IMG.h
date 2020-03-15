#ifndef _IMG_H
#define _IMG_H

#include <string>
#include "GFX.h"

class IMG : public GFX{
	public:
		IMG(std::string, SQLDB*, ILI9488*,bool,bool=true);
		IMG(IMG*, SQLDB*, ILI9488*,bool,bool=true);

		void clear_image(); // Clear IMG

		void render_image(unsigned int, unsigned int);
		
	private:
		

};


#endif