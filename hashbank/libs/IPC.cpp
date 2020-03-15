// Button class -- Inherited from GFX, image object
// Button with behavior when being touched

#include <iostream>
#include <cstring>
#include "IPC.h"

using namespace std;

extern "C"
{
	#include <termios.h> // POSIX terminal control definitionss
	#include <stdio.h> // standard input / output functions
	#include <string.h> // string function definitions
	#include <unistd.h> // UNIX standard function definitions
	#include <fcntl.h> // File control definitions
	#include <errno.h> // Error number definitions
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/types.h>
}

IPC::IPC()
{
	// Default constructor
}

IPC::IPC(string pipe_name)
{
	// Set named pipe ... 
	this->pipe_name = pipe_name;
	// Path of named pipe e.g : /tmp/PIPE_NAME
	this->pipe_path = "/tmp/" + pipe_name; // FIFO name in tmp

	// Create named pipe!
	create_fifo();
}

// Destructor
IPC::~IPC()
{
	// Delete from system!
	delete_fifo();
}

int IPC::create_fifo()
{
	// FIFO Name
	char * F_NAME = new char[pipe_path.length()+1];
	strcpy (F_NAME, pipe_path.c_str());

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    int status = mkfifo(F_NAME, 0666);

	if( status != 0 )
	{
		cout << "Error creating fifo!: " << strerror(errno) << endl;
	}
	else
		cout << "FIFO created succesfully!";

	delete[] F_NAME;
}

void IPC::open_fifo(char open_mode)
{
	this->open_mode = open_mode; // Current open mode

	// FIFO Name
	char * F_NAME = new char[pipe_path.length()+1];
	strcpy (F_NAME, pipe_path.c_str());

	cout << F_NAME << endl;
	// Open FIFO for write only
	if( open_mode == 'r' )
		fd = open(F_NAME, O_RDONLY);
	if( open_mode == 'w' )
		fd = open(F_NAME, O_WRONLY);

	if(fd == -1) // if open is unsucessful
	{
		//perror("open_port: Unable to open /dev/ttyS0 - ");
		cout << "FIFO ERROR: Unable to open named pipe " << pipe_path << " " << strerror(errno) << endl;
	}
	else{
		cout << "FIFO " << pipe_path << " opened succesfully!" << endl;
	}

	delete[] F_NAME;
}

int IPC::write_fifo(ustring data)  
{
	// Named-Pipe not opened on correct mode
	if( open_mode != 'w' )
		return -1;

	// Add Null terminated character (since we are using uint8 and not char)
	uint8_t null_char = 0x00;
	data.push_back(null_char);
  
	int buffer_size = data.size();
	uint8_t * data_write = new uint8_t[buffer_size];
	ustrcpy (data_write, data);

	int n = write(fd, data_write, buffer_size);  //Send data

	// Error Handling 
	int status = 0;
	if (n < 0)
	{
		 cout << "Error Writing: " << strerror(errno) << endl;
		 status = 0;
	}else{
		status = 1;
	}
	
	delete[] data_write;

	return status;
}

int IPC::read_fifo(ustring *data)
{
	// Named-Pipe not opened on correct mode
	if( open_mode != 'r' )
		return -1;

	// Buffer
	int rd_bufsize = 1024; // Read a maximum 1024 bytes at a time
	uint8_t * buf = new uint8_t[rd_bufsize];

	ustring data_received;

	uint8_t nullchar = 0x00;
	bool nullchar_found = false; // Null character found?
	while (!nullchar_found)
	{
		int n = read( fd, buf , rd_bufsize ); // Read!
		// Some bytes were read!
		if (n > 0)
		{			 
			 // Add new data to buffer!
			 for( int i=0; i<n; i++ )
			 {
				if (buf[i] == nullchar)
				{
					// Read up to the null char
					nullchar_found = true;
					break; 
				}
					
				data_received.push_back(buf[i]);
			 }
		}

	}

	// String received
	*data = data_received;

	// Free memory
	delete[] buf;


	return 1;
}

void IPC::close_fifo()
{
	try
	{
		// Close!
		close(fd);
	}
	catch(int e)
	{
		cout << "FIFO closed(del) elsewhere." << endl;
	}
}

void IPC::delete_fifo()
{
	// FIFO Name
	char * F_NAME = new char[pipe_path.length()+1];
	strcpy (F_NAME, pipe_path.c_str());
	
	try
	{
		// Delete!
		// unlink(F_NAME); // Del
	}
	catch(int e)
	{
		cout << "FIFO deleted elsewhere." << endl;
	}

	delete[] F_NAME;
}