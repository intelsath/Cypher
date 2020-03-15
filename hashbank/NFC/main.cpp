#include <iostream>
#include <cstring>

#include "../libs/IPC.h"
#include "../libs/NFC.h"

using namespace std;

int main()
{
	NFC pn7150; // NFC 

	// Poll
	// pn7150.poll();
	// pn7150.write("hello world");

	while(1)
	{
		
		// Use a pipe to listen for commands
		IPC::ustring command;
		IPC named_pipecmd("NFCcommands");
		named_pipecmd.open_fifo('r');
		named_pipecmd.read_fifo(&command);
		string command_received;
		named_pipecmd.vect2str(&command_received, command); // ustring -> string
		named_pipecmd.close_fifo();

		// cmd should be an int, so convert
		int cmds = stoi(command_received);

		// Commands
		switch(cmds)
		{
			// Poll!
			case pn7150.NFC_READ:
			{
				IPC::ustring received_bytes;
				pn7150.poll(&received_bytes);

				// Send to pipe
				IPC pipenfc_contents("NFCcontents");
				pipenfc_contents.open_fifo('w');
				pipenfc_contents.write_fifo(received_bytes);
				pipenfc_contents.close_fifo();

				break;
			}

			// Write tag!
			case pn7150.NFC_WRITE:
			{
				// Use a pipe to listen for commands
				IPC::ustring received_mnemonic;
				IPC named_pipewr("NFCwallet");
				named_pipewr.open_fifo('r');
				named_pipewr.read_fifo(&received_mnemonic);
				string wallet_received;
				named_pipewr.vect2str(&wallet_received, received_mnemonic); // ustring -> string
				named_pipewr.close_fifo();

				cout << "received_mnemonic: " << wallet_received << endl;

				IPC::ustring new_mnemonic;
				int status = pn7150.write(wallet_received, &new_mnemonic); // Write to NFC!
				
				// Send to pipe
				IPC pipenfc_contents("NFCcontents");
				pipenfc_contents.open_fifo('w');
				pipenfc_contents.write_fifo(new_mnemonic);
				pipenfc_contents.close_fifo();


				break;
			}


			// Push message to device!
			case pn7150.NFC_PUSH:
			{
				// Use a pipe to listen for commands
				IPC::ustring received_mnemonic;
				IPC named_pipewr("NFCwallet");
				named_pipewr.open_fifo('r');
				named_pipewr.read_fifo(&received_mnemonic);
				string wallet_received;
				named_pipewr.vect2str(&wallet_received, received_mnemonic); // ustring -> string
				named_pipewr.close_fifo();

				cout << "received_mnemonic: " << wallet_received << endl;

				IPC::ustring new_mnemonic;
				int status = pn7150.write(wallet_received, &new_mnemonic); // Write to NFC!
				
				// Send to pipe
				IPC pipenfc_contents("NFCcontents");
				pipenfc_contents.open_fifo('w');
				pipenfc_contents.write_fifo(new_mnemonic);
				pipenfc_contents.close_fifo();


				break;
			}
			
		}



	}

    return 0;
}
 
