#include <iostream>
#include "fhandler.h"

using namespace std;

// For debugging purposes
void FHANDLER::print_ustring(ustring vectstring)
{
	int bufsize = vectstring.size();

	cout << "0x";
	for(int i=0; i<bufsize;i++)
	{
		printf("%02x",vectstring[i]);
	}
	cout << endl;

}

// Copy the contents of the vector and puts it on a uint8_t array
int FHANDLER::ustrcpy(uint8_t *strout, ustring vectstring)
{
	int bufsize = vectstring.size();
	// cout << "bufsize: " << bufsize << endl;

	if (bufsize > 0)
	{
		for(int i=0; i<bufsize;i++)
		{
			// Copy contents of vector to uint8_t array
			strout[i] = vectstring[i];
		}
		return 1;
	}
	else{
		return 0;
	}

}

// ustring -> string
int FHANDLER::vect2str(string* stringout, ustring stringin)
{
	int bufsize = stringin.size();
	// cout << "bufsize: " << bufsize << endl;

	if (bufsize > 0)
	{
		for(int i=0; i<bufsize;i++)
		{
			// Copy contents of vector to uint8_t array
			stringout->push_back(stringin[i]);
		}
		return 1;
	}
	else{
		return 0;
	}
}

// string -> ustring
int FHANDLER::str2vect(ustring* stringout, string stringin)
{
	int bufsize = stringin.length();
	// cout << "bufsize: " << bufsize << endl;

	if (bufsize > 0)
	{
		for(int i=0; i<bufsize;i++)
		{
			// Copy contents of vector to uint8_t array
			stringout->push_back(stringin[i]);
		}
		return 1;
	}
	else{
		return 0;
	}
}

// vector (big endian format,vect=[...0,0,0,LSB]) -> int number
int FHANDLER::ustoi(ustring vectbytes)
{
	// Convert to string then get the equivalent number
	string strnum;
	string *strnum_ptr = &strnum;
	if( vect2str(strnum_ptr,vectbytes) )
	{
		return stoi(strnum);
	}

}