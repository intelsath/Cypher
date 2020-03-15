#include "NFC.h"

#include <iostream>

using namespace std;

void NFC::getnfc_contents(IPC::ustring* contents)
{
	// poll test!
	IPC named_piper("nfcnci");
	// Open pipe
	named_piper.open_fifo('r');
	// write to FIFO (named pipe)
	named_piper.read_fifo(contents);
	// Close FIFO
	named_piper.close_fifo();
	// Destroy
	// named_piper.~IPC();
}

void NFC::poll(IPC::ustring *received_bytes)
{
	// Run process
	string script_cmd = "LD_LIBRARY_PATH=/usr/local/lib; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/libnfc_nci_linux-1.so.0; export LD_LIBRARY_PATH; ./nfcApp poll & >> nfc_log.txt";
	//string script_cmd = "./nfcApp write --type=text -l en -r \"Hello World\" &";
	char * COIN_CMD = new char[script_cmd.length()+1];
	strcpy (COIN_CMD, script_cmd.c_str());
	cout << "running script: " << COIN_CMD << endl;
	int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess
	// Free memory
	delete[] COIN_CMD;

	// Get contents from pipe
	IPC nfccontents;
	IPC::ustring nfc_poll;
	getnfc_contents(&nfc_poll);
	cout << "Contents: " << endl;
	nfccontents.print_ustring(nfc_poll);

	// Return bytes
	for( int i=0; i<nfc_poll.size(); i++ )
		received_bytes->push_back(nfc_poll[i]);
}

int NFC::write(string data, IPC::ustring *new_wallet)
{
	// Run process
	string script_cmd = "LD_LIBRARY_PATH=/usr/local/lib; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/libnfc_nci_linux-1.so.0; export LD_LIBRARY_PATH; ./nfcApp write --type=text -l en -r \"" + data + "\" & >> nfc_log.txt";
	char * COIN_CMD = new char[script_cmd.length()+1];
	strcpy (COIN_CMD, script_cmd.c_str());
	cout << "running script: " << COIN_CMD << endl;
	int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess
	// Free memory
	delete[] COIN_CMD;

	IPC nfccontents;
	// Get contents from pipe
	IPC::ustring nfc_poll;
	getnfc_contents(&nfc_poll);
	cout << "Old Contents: " << endl;
	nfccontents.print_ustring(nfc_poll);

	// Str nfc_contents
	string nfc_oldcontents;
	nfccontents.vect2str(&nfc_oldcontents, nfc_poll); // ustring -> string
	if( nfc_oldcontents == "ERROR" )
	{
		// Return bytes
		for( int i=0; i<nfc_poll.size(); i++ )
			new_wallet->push_back(nfc_poll[i]);

		return 0;
	}
	
	// Get contents from pipe
	IPC::ustring nfc_poll_new;
	getnfc_contents(&nfc_poll_new);
	cout << "Contents: " << endl;
	nfccontents.print_ustring(nfc_poll_new);

	// Return bytes
	for( int i=0; i<nfc_poll_new.size(); i++ )
		new_wallet->push_back(nfc_poll_new[i]);
	
	return 1;
	
}


int NFC::push(string data, IPC::ustring *new_wallet)
{
	// Run process
	string script_cmd = "LD_LIBRARY_PATH=/usr/local/lib; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/libnfc_nci_linux-1.so.0; export LD_LIBRARY_PATH; ./nfcApp push -t text -l en -r  \"" + data + "\" & >> nfc_log.txt";
	char * COIN_CMD = new char[script_cmd.length()+1];
	strcpy (COIN_CMD, script_cmd.c_str());
	cout << "running script: " << COIN_CMD << endl;
	int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess
	// Free memory
	delete[] COIN_CMD;

	IPC nfccontents;
	// Get contents from pipe
	IPC::ustring nfc_poll;
	getnfc_contents(&nfc_poll);
	cout << "Old Contents: " << endl;
	nfccontents.print_ustring(nfc_poll);

	// Str nfc_contents
	string nfc_oldcontents;
	nfccontents.vect2str(&nfc_oldcontents, nfc_poll); // ustring -> string
	if( nfc_oldcontents == "ERROR" )
	{
		// Return bytes
		for( int i=0; i<nfc_poll.size(); i++ )
			new_wallet->push_back(nfc_poll[i]);

		return 0;
	}
	
	// Get contents from pipe
	IPC::ustring nfc_poll_new;
	getnfc_contents(&nfc_poll_new);
	cout << "Contents: " << endl;
	nfccontents.print_ustring(nfc_poll_new);

	// Return bytes
	for( int i=0; i<nfc_poll_new.size(); i++ )
		new_wallet->push_back(nfc_poll_new[i]);
	
	return 1;
	

}