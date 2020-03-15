#ifndef _FHANDLER_H
#define _FHANDLER_H

#include <vector>
#include <cstring>
#include <string>

class FHANDLER{
	public:
		// FHANDLER();

		// Vector container handlers
		typedef std::vector<uint8_t> ustring; // ustring = vector containing a bunch of uint8_t elements
		void print_ustring(ustring); // Basically show contents of vector (hex)
		int ustrcpy(uint8_t*,ustring); // Copy string to a uint8_t 
		int vect2str(std::string*, ustring); // Convert ustring (aka vector<uint8_t>) to string
		int str2vect(ustring*, std::string); // Convert string to ustring(aka vector<uint8_t>)
		int ustoi(ustring); // stoi equivalent for ustring type (vector to int number)

		
	protected:
		int fd; // File descriptor
};


#endif