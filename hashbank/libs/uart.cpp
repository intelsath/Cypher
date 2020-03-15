// Uart: https://www.cmrr.umn.edu/~strupp/serial.html
// http://www.cplusplus.com/forum/beginner/6914/

/////////////////////////////////////////////////
// Serial port interface program               //
/////////////////////////////////////////////////

extern "C" {
	#include <termios.h> // POSIX terminal control definitionss
	#include <stdio.h> // standard input / output functions
	#include <string.h> // string function definitions
	#include <unistd.h> // UNIX standard function definitions
	#include <fcntl.h> // File control definitions
	#include <errno.h> // Error number definitions
	#include <time.h>   // time calls
	#include <stdlib.h>
}

#include <iostream>
#include <ctime>
#include <fstream>    
#include <cstring>
#include <cstdlib> 
#include <bitset>

#include "../libs/uart.h"
#include "../libs/IPC.h"

using namespace std;

UART::UART(){
	configure_port();
	CalculateCrcTable_CRC32(); // Define lookup table for CRC

	tx_bufsize = defaulttxrx_buffer; // Buffer size 
	rx_bufsize = defaulttxrx_buffer; // Buffer size 

	// HB Commands
	HB_COMMANDS["HB_HNDSHK"] = {0x48,0x42,0x5F,0x48,0x4E,0x44,0x53,0x48,0x4B};
	HB_COMMANDS["HB_INFOOK"] = {0x48,0x42,0x5F,0x49,0x4E,0x46,0x4F,0x4F,0x4B};
	HB_COMMANDS["HB_RSNDTA"] = {0x48,0x42,0x5F,0x52,0x53,0x4E,0x44,0x54,0x41}; // Request data to be resent

	//HB_COMMANDS["PC_INFOOK"] = {0x50,0x43,0x5F,0x49,0x4E,0x46,0x4F,0x4F,0x4B}; // The info that got to PC was ok!
	HB_COMMANDS["PC_RSNDTA"] = {0x50,0x43,0x5F,0x52,0x53,0x4E,0x44,0x54,0x41}; // Request data to be resent

	// PC Commands (See enum COMMANDS_ENMAP{} on class declaration) -- aka Main comamnds because PC is master, HB is slave -
	PC_COMMANDS["PC_HNDSHK"] = {0x50,0x43,0x5F,0x48,0x4E,0x44,0x53,0x48,0x4B}; COMMANDS_MAP["PC_HNDSHK"] = 0; // Used for data OK and initial handshake
	PC_COMMANDS["PC_SNDFLE"] = {0x50,0x43,0x5F,0x53,0x4E,0x44,0x46,0x4C,0x45}; COMMANDS_MAP["PC_SNDFLE"] = 1; // Send file from PC to HB
	// PC_COMMANDS["PC_RCVFLE"] = {0x50,0x43,0x5F,0x52,0x43,0x56,0x46,0x4C,0x45}; COMMANDS_MAP["PC_RCVFLE"] = 2; // Receive file from HB to PC
	PC_COMMANDS["PC_CRTADR"] = {0x50,0x43,0x5F,0x43,0x52,0x54,0x41,0x44,0x52}; COMMANDS_MAP["PC_CRTADR"] = 2; // Create new address for a specific coin
	PC_COMMANDS["PC_RQSADR"] = {0x50,0x43,0x5F,0x43,0x52,0x54,0x41,0x44,0x52}; COMMANDS_MAP["PC_RQSADR"] = 3; // Request all addresses of a specific coin
	PC_COMMANDS["PC_CRTWLT"] = {0x50,0x43,0x5F,0x43,0x52,0x54,0x57,0x4C,0x54}; COMMANDS_MAP["PC_CRTWLT"] = 4; // Create new wallet
	PC_COMMANDS["PC_SGNTRN"] = {0x50,0x43,0x5F,0x53,0x47,0x4E,0x54,0x52,0x4E}; COMMANDS_MAP["PC_SGNTRN"] = 5; // Sign transaction
	PC_COMMANDS["PC_NFCRAD"] = {0x50,0x43,0x5F,0x4E,0x46,0x43,0x52,0x41,0x44}; COMMANDS_MAP["PC_NFCRAD"] = 6; // Read NFC wallet
	PC_COMMANDS["PC_NFCWRT"] = {0x50,0x43,0x5F,0x4E,0x46,0x43,0x57,0x52,0x54}; COMMANDS_MAP["PC_NFCWRT"] = 7; // Write NFC wallet
	PC_COMMANDS["PC_RSTRWL"] = {0x50,0x43,0x5F,0x52,0x53,0x54,0x52,0x57,0x4C}; COMMANDS_MAP["PC_RSTRWL"] = 8; // Restore wallet via NFC backup
	PC_COMMANDS["PC_UPDATE"] = {0x50,0x43,0x5F,0x55,0x50,0x44,0x41,0x54,0x45}; COMMANDS_MAP["PC_UPDATE"] = 9; // Update wallet software
	
	expecting_rx = 0;
	last_data_corrrect = 0;

	initial_handshake_sending = false;
}

UART::~UART(){
	close_port();
}

// configure the port
int UART::configure_port()      
{
	struct termios port_settings;      // structure to store the port settings in
	// Open ttys4
	fd = open("/dev/ttyS4", O_RDWR | O_NOCTTY );

	if(fd == -1) // if open is unsucessful
	{
		//perror("open_port: Unable to open /dev/ttyS0 - ");
		printf("open_port: Unable to open /dev/ttyS4. \n");
	}
	else
	{
		tcgetattr(fd, &port_settings);
		fcntl(fd, F_SETFL, 0);
		/* get the current options */
		printf("port is open.\n");
	
	
		cfsetispeed(&port_settings, B115200);    // set baud rates
		cfsetospeed(&port_settings, B115200);

		port_settings.c_cflag |= (CLOCAL | CREAD);  /* ignore modem controls */
		port_settings.c_cflag &= ~CSIZE;
		port_settings.c_cflag |= CS8;           /* 8-bit characters */
		port_settings.c_cflag &= ~PARENB;       /* no parity bit */
		port_settings.c_cflag &= ~CSTOPB;       /* only need 1 stop bit */
		port_settings.c_cflag &= ~CRTSCTS;  /* no hardware flowcontrol */

		/* setup for non-canonical mode */
		port_settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		port_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		port_settings.c_oflag &= ~OPOST;

		/* fetch bytes as they become available */
		port_settings.c_cc[VMIN] = 0;
		port_settings.c_cc[VTIME] = 0;
		 
	
		tcsetattr(fd, TCSANOW, &port_settings);    // apply the settings to the port
	}
	return(fd);
} 

// Lookup table
void UART::CalculateCrcTable_CRC32(){
    unsigned long POLYNOMIAL = 0xEDB88320; // Reversed polynomial (0x04C11DB7)
    unsigned long remainder;
    unsigned char b = 0;

    do {
        // Start with the data byte
        remainder = b;
        for (unsigned long bit = 8; bit > 0; --bit) {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            else
                remainder = (remainder >> 1);
        }
        crcTable[(size_t)b] = remainder;
    } while(0 != ++b);
}

// CRC
uint32_t UART::Compute_CRC32(ustring p) {
    uint32_t crc = 0xffffffff; // 32-bit CRC init
    size_t i;
	int byteslen = p.size();
    for(int i = 0; i < byteslen; i++)
        crc = crcTable[p[i] ^ (crc&0xff)] ^ (crc>>8);
    return(~crc);
}


 // Write to serial port
int UART::uart_write(ustring data)  
{
	int buffer_size = data.size();
	uint8_t * data_write = new uint8_t[buffer_size];
	ustrcpy (data_write, data);

	int n = write(fd, data_write, buffer_size);  //Send data

	//usleep(1000);
	//tcdrain(fd);

	// cout << "Data wrote: " << endl;
	// print_ustring(data);

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

int UART::uart_read(ustring *data,int buffer_size)
{
	// Buffer
	int rd_bufsize = buffer_size*2; // Add space for any leftover bytes
	uint8_t * buf = new uint8_t[rd_bufsize];

	// Flush contents of the serial port
	//tcflush(fd, TCIOFLUSH);
	//usleep(1000);

	ustring data_received;
	// Read
	time_t t1 = time(0);  // t is an integer type
	int read_valid = true; // Assume it's going to be valid
	int n_bytes = 0;
	// Timeout
	int timeout = 0;

	// Estahlish timeout based if we are sending handshakes (initial) or not
	// This makes software (app) faster to find "match" through serial port
	if (initial_handshake_sending == true)
		timeout = 1;
	else
		timeout = 60;

	while (n_bytes < buffer_size)
	{
		time_t t2 = time(0);  // t is an integer type
		int n = read( fd, buf , rd_bufsize );

		// Some bytes were read!
		if (n > 0)
		{
			 // cout << "Number of bytes received: " << n << endl;
			 t1 = time(0);  // reset timer (because its currently reading correctly)
			 n_bytes+=n;
			 
			 // Add to buffer new data!
			 for( int i=0; i<n; i++ )
			 {
				//cout << "buf" << i << ": ";
				//printf("%02x",buf[i]);
				//cout << endl;

				if (i >= buffer_size)
					break; // Only read within the bounds
				data_received.push_back(buf[i]);
			 }
		}
		
		time_t epoch = t2-t1;

		//cout << "Epoch: " << epoch << endl;
		//cout << "expecting_rx: " << expecting_rx << endl;
		// Have we been reading for the past timeout secs? If not break from loop!
		if(epoch >= timeout)
		{
			// Epoch (timeout of n seconds)
			read_valid = false; // No valid data received yet!
			
			if( expecting_rx ) // Are we expecting new data now?
			{
				cout << "Timeout reached... Resending handshake... Status of last data was " << last_data_corrrect << endl;
				// No data has been received yet? Resend handshake!
				if( last_data_corrrect )
					data_validate();   // Last data was correct, so resent handshake
				else
					request_repeatdata();  // Last data was corrupted, so request last data again
				
			}
				
			break;
		}
			
	}

	
	if( read_valid )
	{
		// String received
		*data = data_received;
		// cout << "Data received..." << endl;
		// print_ustring(data_received);
		
	}


	delete[] buf;

	return read_valid;
	
}

// Initial Handhsake
void UART::first_handshake()
{
	initial_handshake_sending = true;
	cout << "First handshake initiating..." << endl;
	// Write string
	ustring wrstr = HB_COMMANDS["HB_HNDSHK"];
	// communicate TX HB protocol
	uart_txcommunicate(wrstr);
	cout << "Finished first handshake!" << endl;
	initial_handshake_sending = false;
}

// Read and verify if CRC is correct
int UART::uart_receive_data(ustring *rxString){

	const int rx_newbufsize = rx_bufsize + CRC_NBYTES;
	// String pointer
	ustring data;
	ustring *data_ptr = &data;

	int rxstatus = uart_read(data_ptr,rx_newbufsize);

	// cout << "rxstatus: " << rxstatus << endl;

	if( rxstatus ) // Verify CRC
	{
		rxstatus = verify_data(data,rx_bufsize,rxString);
		// cout << "verified data status: " << rxstatus << endl;
	}

	return rxstatus;
}

// Write to serial port and 4-byte CRC
int UART::uart_transfer_data(ustring write_string){

	// string pointer
	ustring data_wcrc;
	ustring *data_ptr = &data_wcrc;
	append_crc(write_string,data_ptr);

	// Transfer to serial port
	int tx_status = uart_write(data_wcrc);
	//cout << "tx status: " << tx_status << endl;

	if( tx_status )
	{
		return 1;
	}else{
		return 0;
	}
}

// Append the 4 bytes to the end of the string
void UART::append_crc(ustring write_string,ustring *appendedcrc_string)
{
	// Compute CRC32
	uint32_t crc = Compute_CRC32(write_string);
	int bufsize = write_string.size();

	ustring apnded_str; 
	// Append CRC to string to send
	for (int i=0; i<bufsize; i++){
		apnded_str.push_back(write_string[i]); // "Append" 1 char
	}
	
	// Split bits
	for( int i=CRC_NBYTES; i > 0; i-- )
	{
        uint32_t byte_chunkn = crc>>((i-1)*8); // Shift the bits 
        char byte1 = byte_chunkn;		// Get the LSBs because char = 1 byte... This can also be done with a "moving" mask e.g 0xFF000000.
        
		// Append to array
        apnded_str.push_back(byte1); // "Append" 1 char
	}

	// Appended string w/ crc32
	*appendedcrc_string = apnded_str;
}

// Verify data using CRC32
int UART::verify_data(ustring received_data, int original_bufsize, ustring *rxDATA){
	
	uint32_t fullrcv_CRC = 0x00000000;

	ustring rx_dta; // Data without CRC
	// Get the last 4 bytes to get checksum
	for (int i=original_bufsize; i<original_bufsize+CRC_NBYTES; i++) {
		fullrcv_CRC <<= 8;
		fullrcv_CRC |= received_data[i];
	}

	// Get actual data
	for(int i=0; i<original_bufsize; i++ )
	{
		rx_dta.push_back(received_data[i]);		// Output data
	}
	*rxDATA = rx_dta;

	// Compute CRC32
	uint32_t new_CRC = Compute_CRC32(rx_dta);

	// cout << "new_CRC: " << new_CRC << endl;
	// cout << "fullrcv_CRC: " << fullrcv_CRC << endl;

	if( new_CRC == fullrcv_CRC){
		//cout << "CRC passed..!" << endl;
		return 1;  // success!
	}else{
		//cout << "CRC error!" << endl;
		
		return -1; // Error detected in received data!
	}

}

// -------------------------------------------------------- 
// HB UART protocol
// -------------------------------------------------------- 
// Compare with commands
int UART::compare_received_string(ustring rx_string, ustring expected_command)
{
	int cmd_compare = 0;

	if( rx_string ==  expected_command)
		cmd_compare = 1;

	return cmd_compare;
}

// Send something and expect a handshake
// Data will be sent indefinitely in a loop until we get a handhsake
void UART::uart_txcommunicate(ustring wrstr)
{
	
	int verified_str = false; // Did we get a handshake?
	while(!verified_str){
		
		// uart transmit data
		int tx_status = uart_transfer_data(wrstr);
		if ( tx_status ){
			// cout << "Data was sent succesfully!" << endl;
		}
		else{
			cout << "Error when sending data to serial port." << endl;
		}
		
		ustring rxString;
		ustring *data_ptr = &rxString;
		int rx = uart_receive_data(data_ptr);
		
		// If I received something
		if(rx){
			// cout << "Valid data received!" << endl;
			// cout << "String received: ";
			// print_ustring(rxString);

			// Did we get a PC_HNDSHK telling us that the last sent data was received ok?
			verified_str = compare_received_string(rxString,PC_COMMANDS["PC_HNDSHK"]);

			// cout << "verified_str: " << verified_str << endl;
		}
	}
}

// Expect some data (use this after we KNOW for sure PC will be sending some data to HB)
void UART::uart_rxcommunicate(ustring * received_string)
{
	// cout << "RX COM... " << endl;
	int rx = 0;
	while(rx <= 0) // If it/s -1 or 0 
	{
		expecting_rx = 1; // Expecting data
		// cout << "changing value of expecting_rx: " << expecting_rx << endl;
		ustring rxString;
		ustring *rxString_ptr = &rxString;

		rx = uart_receive_data(rxString_ptr);
		
		// If I received something
		if(rx == 1)
		{
			data_validate();   // Data was OK
			*received_string = rxString;
		}
		else if( rx == -1 ){ // STATUS =2 means received something but invalid CRC
			
			request_repeatdata(); // Data was corrupted. Request data to be re-sent

		}
	}

	expecting_rx = 0;
	
}

void UART::send_handshake()
{
	ustring wrstr = HB_COMMANDS["HB_HNDSHK"];

	// uart transmit data
	int tx_status = uart_transfer_data(wrstr);
	if ( tx_status ){
		//cout << "Handshake was sent succesfully!" << endl;
	}
	else{
		//cout << "Error when sending Handshake to serial port." << endl;
	}

}

void UART::data_validate()
{
	ustring wrstr = HB_COMMANDS["HB_INFOOK"];

	// uart transmit data
	int tx_status = uart_transfer_data(wrstr);
	if ( tx_status ){
		//cout << "INFOOK was sent succesfully!" << endl;
	}
	else{
		//cout << "Error when sending Handshake to serial port." << endl;
	}

	// Last data received was OK
	last_data_corrrect = 1;
}

void UART::request_repeatdata()
{
	ustring wrstr = HB_COMMANDS["HB_RSNDTA"];

	int tx_status = uart_transfer_data(wrstr);
	if ( tx_status ){
		cout << "HB_RSNDTA sent!" << endl;
	}
	else{
		//cout << "Error when sending HB_RSNDTA to serial port." << endl;
	}

	// Last data received was corrupted
	last_data_corrrect = 0; 
}

void UART::changerx_buffer()
{
	//cout << "Expecting new buffer..." << endl;
	ustring expectedbuf;
	ustring *expectedbuf_ptr = &expectedbuf;
	uart_rxcommunicate(expectedbuf_ptr);   // Receive new buffer
	rx_bufsize = ustoi(expectedbuf);   // RX buffer 
	//cout << "Expected new buffer: " << rx_bufsize << endl; 
}

UART::ustring UART::changetx_buffer(string str2send) 
{
	// Make sure there are 9 (default buffer) characters
	int strsize = str2send.length();
	string nxt_buffer = to_string(strsize);

	int left_chrs = defaulttxrx_buffer-nxt_buffer.length(); // characters to fill

	string concat_chrs;
	// Add zeroes to fill all the gap
	for( int i=0; i<left_chrs; i++ )
	{
		concat_chrs += "0";
	}
	// New buffer size (with appended zeroes)
	nxt_buffer = concat_chrs + nxt_buffer;

	// Conversion between string and ustring
	ustring ustr_nxt_buffer;
	ustring *ustr_ptr_nxt_buffer = &ustr_nxt_buffer;
	
	// Get new buffer size in ustring format
	str2vect(ustr_ptr_nxt_buffer, nxt_buffer);

	return ustr_nxt_buffer;
}

void UART::defaultrx_buffer()
{
	rx_bufsize = defaulttxrx_buffer;
}

void UART::defaulttx_buffer()
{
	tx_bufsize = defaulttxrx_buffer;
}

// Receive file through UART
void UART::uart_receive_file()
{
	cout << "Receiving file..." << endl;

	// Get expected buffer size for the next iteration
	changerx_buffer();

	// Filename
	ustring ustrfname;
	ustring *ustrfname_ptr = &ustrfname;
	uart_rxcommunicate(ustrfname_ptr);
	// Get contents in string format
	string filename;
	string *filename_ptr = &filename;
	vect2str(filename_ptr, ustrfname); // ustring -> string
	cout << "Filename: " << filename << endl;

	char * F_NAME = new char[filename.length()+1];
	strcpy (F_NAME, filename.c_str());

	// Open file for output in binary mode
	ofstream outfile ((const char*)F_NAME,ios::out | ios::binary);

	// Receive file
	while( rx_bufsize != 0 )
	{
		// Set buffer to default again
		defaultrx_buffer(); 
		// Get expected NEW buffer size for the next iteration
		changerx_buffer();

		if( rx_bufsize == 0) // Next buffer is zero, so we finished reading file
			break;

		// Read data
		ustring ustrfbuffer;
		ustring *ustrfbuffer_ptr = &ustrfbuffer;
		uart_rxcommunicate(ustrfbuffer_ptr);
		// Get contents in string format
		//string file_buffer;
		//string *file_buffer_ptr = &file_buffer;
		//vect2str(file_buffer_ptr, ustrfbuffer); // ustring -> string

		uint8_t * F_BUFFER = new uint8_t[ustrfbuffer.size()];
		ustrcpy (F_BUFFER, ustrfbuffer);

		// write to outfile
		outfile.write ((const char*)F_BUFFER,rx_bufsize);
		// outfile << F_BUFFER;

		delete[] F_BUFFER; // Free memory

	}

	cout << "File transfer completed! " << endl;
	outfile.close(); // Close file
	// Set buffer to default again
	defaultrx_buffer(); 

	// Free memory
	delete[] F_NAME;

	// Resets variables 
	expecting_rx = 0;
	last_data_corrrect = 0;
}


// Close
void UART::close_port(){
	close(fd);
}