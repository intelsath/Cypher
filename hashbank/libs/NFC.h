// NFC
#ifndef _NFC_H
#define _NFC_H

#include "IPC.h"
#include <vector>

class NFC{
	public:
		void getnfc_contents(IPC::ustring*);  // Get contents from pipe!
		void poll(IPC::ustring *);
		int write(std::string, IPC::ustring *); // Write to NFC tag
		int push(std::string, IPC::ustring *); // Push message to device

		enum COMMANDS_ENMAP{NFC_READ,NFC_WRITE,NFC_PUSH};
	private:
		
		// static ustring received_message;
		// static std::vector<unsigned char> message_received;
};

#endif