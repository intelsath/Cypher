
#include <iostream>

#include "../libs/IPC.h"

using namespace std;

int main ()
{
	cout << "Creating pipe..." << endl;
	// Prepare UART
	IPC named_pipe("rpipe1");
	cout << "done!" << endl;

	// ------------------------------------------
	// Write
	cout << "Opening pipe..." << endl;
	// Open pipe
	named_pipe.open_fifo('w');
	cout << "done!" << endl;
	string data_str = "Hello World!";
	IPC::ustring data_write;
	named_pipe.str2vect(&data_write,data_str);
	cout << "Writing to pipe..." << endl;
	// Write to pipe!
	named_pipe.write_fifo(data_write);
	// Close!
	named_pipe.close_fifo();
	cout << "done!" << endl;

	// ------------------------------------------
	// Read
	// Open pipe
	/*cout << "Reading pipe..." << endl;
	named_pipe.open_fifo('r');
	IPC::ustring data_read;
	// Read fifo
	named_pipe.read_fifo(&data_read,12);
	// Print
	cout << "Data Received: " << endl;
	named_pipe.print_ustring(data_read);
	// Close!
	named_pipe.close_fifo();
	cout << "done!" << endl;*/

	return 0;

}