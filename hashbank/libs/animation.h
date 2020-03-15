#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <string>
#include "GFX.h"
#include "stopwatch.h"

class ANIMATION : public GFX{
	public:
		ANIMATION(std::string, SQLDB*, ILI9488*, bool, unsigned int, unsigned int);
		// ANIMATION(ANIMATION*, SQLDB*, ILI9488*, bool, unsigned int);

		void init(unsigned int, unsigned int); // This will render image for the first time
		void animate(unsigned int, unsigned int);
		
	private:
		STOPWATCH timer;
		
		// Note: TOtalframes = size of vector BITMAP_HOLDER at GFX
		unsigned int frame_itr; // Frame iterator
		unsigned int fps;
		double secs; // Inverse of fps in secs 

		bool initialized;
};


#endif