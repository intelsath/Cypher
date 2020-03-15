// Uart: https://www.cmrr.umn.edu/~strupp/serial.html
// http://www.cplusplus.com/forum/beginner/6914/

extern "C"{
	#include <stdio.h> 
	#include <stdint.h> 
}
#include <iostream>
#include <bitset>

#include "libs/uart.h"

using namespace std;

int main()
{ 
	UART uart_connection;

	// Buffer size of 1 byte
	int bufsize = 5;
	unsigned char write_string[] = "Hi!\r\n"; 
	
	// uart transmit data
	int tx_status = uart_connection.uart_transfer_data(write_string,bufsize);
	if ( tx_status ){
		cout << "Data was sent succesfully!" << endl;
	}
	else{
		cout << "Error when sending data to serial port." << endl;
	}
	
	//unsigned char *write_string = new unsigned char[bufsize+1];
	/*
	cout << "Original: ";
	for( int i=0; i < bufsize; i++ )
	{
		cout << bitset<8>(write_string[i]);
	}
	cout << endl;

	unsigned char *appendedcrc_string;
	uart_connection.append_crc(write_string,appendedcrc_string,bufsize);
	cout << "Appended: ";
	for( int i=0; i < 4+bufsize; i++ )
	{
		cout << bitset<8>(appendedcrc_string[i]);
	}
	cout << endl;


	int data_status = uart_connection.verify_data(appendedcrc_string, bufsize);

	if( data_status ){
		cout << "Data was received succesfully!" << endl;
	}else{
		cout << "Error detected!" << endl;
	}
	*/

	return 0;
} 