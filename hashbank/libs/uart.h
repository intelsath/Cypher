#ifndef _UART_H
#define _UART_H

#include <string>
#include <vector>
#include <cstdio>
#include <tuple>
#include <map>

// Some functions inherited from FHANDLER
#include "fhandler.h"

class UART : public FHANDLER{
	
	public:
		UART();
		~UART();

		int configure_port(); // All port configurations such as parity, baud rate, hardware flow, etc
		int uart_write(ustring);     // Send characters to the serial port
		int uart_read(ustring*, int);			 // Read from serial port
		// Close
		void close_port();

		// CRC
		void CalculateCrcTable_CRC32();
		uint32_t Compute_CRC32(ustring);
		void append_crc(ustring,ustring*);
		int verify_data(ustring,int,ustring*);

		// Basically same functions as uart_write and uart_read but with CRC implementation
		int uart_transfer_data(ustring); // write
		int uart_receive_data(ustring*);  // read (returns a string)

		// UART HB protocol
		void first_handshake(); // establish first communication
		int compare_received_string(ustring,ustring);
		void uart_txcommunicate(ustring); // Send data with the HB protocol (resend data until we get PC handshake)  
		void uart_rxcommunicate(ustring*); // Receive data with the HB protocol (expect data and send handshake when receiving data proved to be ok)
		void request_repeatdata(); // Wrong data sent, request data to be resent
		void send_handshake();
		void data_validate(); // Send infook
		void changerx_buffer();    // Change rx buffer
		ustring changetx_buffer(std::string);    // Change rx buffer 
		void defaultrx_buffer(); // Change rx buffer to default
		void defaulttx_buffer(); // Change tx buffer to default
		// Some basic main functions
		void uart_receive_file(); // Receive a file through UART

		int get_txbufsize(){ return tx_bufsize; }
		int get_rxbufsize(){ return rx_bufsize; }
		int get_maxbuffer(){ return MAX_TXBUFFER; }

		// Commands
		std::map<std::string,ustring> HB_COMMANDS;
		std::map<std::string,ustring> PC_COMMANDS;
		std::map<std::string,int> COMMANDS_MAP; // Map all commands to an int
		// Commands
		enum COMMANDS_ENMAP{PC_HNDSHK,PC_SNDFLE,PC_CRTADR,PC_RQSADR,PC_CRTWLT,PC_SGNTRN,PC_NFCRAD,PC_NFCWRT,PC_RSTRWL,PC_UPDATE}; // UART commands
		enum NFC_ENMAP{NFC_READ,NFC_WRITE}; // NFC commands

	private:
		int tx_bufsize, rx_bufsize; // Buffer size
		unsigned int crcTable[256]; // Lookup table for CRC
		static const int CRC_NBYTES = 4; // Bytes of the CRC algorithm (32 bits)

		static const int defaulttxrx_buffer = 9;
		static const int MAX_TXBUFFER = 1024; // Max TX buffer

		int expecting_rx; // Are we currently expecting data?
		int last_data_corrrect; // was the last data received correct?

		bool initial_handshake_sending; // Are we current IN menu? aka sending hankshakes?
};

#endif