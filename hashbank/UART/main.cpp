// Uart: https://www.cmrr.umn.edu/~strupp/serial.html
// http://www.cplusplus.com/forum/beginner/6914/

/* NOTE -reminder-: Some amount of RAM is needed to sign transactions. Put a warning in the app to not sign a really big transaction in a single transaction. e.g 50MB ...See switch case 5...*/

extern "C"{
	#include <stdio.h> 
	#include <stdint.h> 
}

#include <iostream>
#include <cmath>
#include <string>
#include <cstring>
#include <map>

#include "../libs/uart.h"
#include "../libs/sqldb.h"
#include "../libs/IPC.h"

using namespace std;

// Page example:
// e.g page=1 : create_address, page=2 : backup wallet, etc etc
// Show to LCD what's being done through USB
void send_usbpage_lcd(int page, UART* uart_connection)
{
	UART::ustring usb_page; // ustring to send to pipe
	string current_usbpage = to_string(page); // Current page!
	uart_connection->str2vect(&usb_page, current_usbpage);
	IPC named_pipecmd("USBcommands");
	named_pipecmd.open_fifo('w');
	named_pipecmd.write_fifo(usb_page);
	named_pipecmd.close_fifo();
}

int main()
{ 
	// Prepare UART
	UART uart_connection;

	// SQL database object 
	SQLDB hashbankdb("../cypher.db");
	
	// NOTE: Order taken from db
	/*map<string,int> COIN_MAP; // Coin map
	COIN_MAP["Bitcoin"] = 1; COIN_MAP["BitcoinCash"] = 2; COIN_MAP["Litecoin"] = 3;
	COIN_MAP["Ethereum"] = 4; COIN_MAP["Ripple"] = 5;*/

	while(1)
	{
		// Initial handshake
		uart_connection.first_handshake();

		// cout << "Getting into main..." << endl;
		UART::ustring ustrrx;
		UART::ustring *ustrrx_ptr = &ustrrx;
		int rx = uart_connection.uart_receive_data(ustrrx_ptr);
		// Get contents in string format
		string rxString;
		string *rxString_ptr = &rxString;
		uart_connection.vect2str(rxString_ptr, ustrrx); // ustring -> string
		if(rx){
			
			int cmds = -1;
			try
			{
				cmds = uart_connection.COMMANDS_MAP[rxString];
			}
			catch (const char* msg)
			{
				cout << "Unexpected command! " << msg << endl;
				continue; // Nothing to do here, wrong command
			}

			uart_connection.data_validate(); // Correct data received, send handshake

			// All available commands 
			switch( cmds )
			{
				// Send file from PC to HB
				case uart_connection.PC_SNDFLE:
				{
					//send_usbpage_lcd(1, &uart_connection); // Send page number to lcd.service!
					uart_connection.uart_receive_file();
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!
					break;
				}

				// Create new address for a specific cryptocurrency
				case uart_connection.PC_CRTADR:
				{
					//send_usbpage_lcd(2, &uart_connection); // Send page number to lcd.service!
					cout << "Receiving coin name..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();

					// Coin name
					UART::ustring ustrcname;
					UART::ustring *ustrcname_ptr = &ustrcname;
					uart_connection.uart_rxcommunicate(ustrcname_ptr);
					// Get contents in string format
					string coinname;
					string *coinname_ptr = &coinname;
					uart_connection.vect2str(coinname_ptr, ustrcname); // ustring -> string
					cout << "Coinname: " << coinname << endl;

					// Default bufsize
					uart_connection.defaultrx_buffer();

					cout << "Receiving wallet id..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();

					// Wallet id
					UART::ustring ustrwid;
					UART::ustring *ustrwid_ptr = &ustrwid;
					uart_connection.uart_rxcommunicate(ustrwid_ptr);
					// Get contents in string format
					string wallet_id;
					string *wallet_id_ptr = &wallet_id;
					uart_connection.vect2str(wallet_id_ptr, ustrwid); // ustring -> string
					cout << "Wallet id: " << wallet_id << endl;

					// Default bufsize
					uart_connection.defaultrx_buffer();

					// Run script to create new address
					string script_cmd = "nodejs ../cryptolibs/" + coinname + "_generateKey.js " + wallet_id;
					char * COIN_CMD = new char[script_cmd.length()+1];
					strcpy (COIN_CMD, script_cmd.c_str());
					cout << "running script: " << COIN_CMD << endl;
					int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess
					// Free memory
					delete[] COIN_CMD;

					UART::ustring nxt_bufsize; // buffer size
					// Script ran succesfully
					if( !script_status )
					{
						// Addresses
						string public_key;
						string public_address;

						string sqlqry  = "SELECT public_key,address FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname + "') AND wallet_id = " + wallet_id + " ORDER BY ID DESC LIMIT 1;";
						int returned_records = hashbankdb.sql_query(sqlqry); // Records returned

						// Retrieve values from db
						if( returned_records > 0 )
						{
							public_key = hashbankdb.sql_vals[0];
							public_address = hashbankdb.sql_vals[1];
						}

						cout << "Public Key: " << public_key << endl;
						cout << "Public Address: " << public_address << endl;

						// Send public key through serial port
						
						UART::ustring public_key_tx;
						// Buffer size of public key
						nxt_bufsize = uart_connection.changetx_buffer(public_key);
						// Convert string to ustring
						uart_connection.str2vect(&public_key_tx, public_key);
						// Send buffer
						cout << "sending next buf..." << endl;
						uart_connection.uart_txcommunicate(nxt_bufsize);
						cout << "sending public key..." << endl;
						// Send actual contents
						uart_connection.uart_txcommunicate(public_key_tx);

						// Default bufsize
						uart_connection.defaulttx_buffer();

						// Send public address through serial port
						UART::ustring public_address_tx;
						// Buffer size of public key
						nxt_bufsize = uart_connection.changetx_buffer(public_address);
						// Convert string to ustring
						uart_connection.str2vect(&public_address_tx, public_address);
						// Send buffer
						cout << "sending next buf..." << endl;
						uart_connection.uart_txcommunicate(nxt_bufsize);
						cout << "sending public address..." << endl;
						// Send actual contents
						uart_connection.uart_txcommunicate(public_address_tx);

						// Send bufsize 0f 0
						//nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send
						// Convert string to ustring
						//uart_connection.uart_txcommunicate(nxt_bufsize);
					}
					else{
						// Error, couldn't create new address!
					}

					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!
					break;
				}

				case uart_connection.PC_RQSADR:
				{
					//send_usbpage_lcd(3, &uart_connection); // Send page number to lcd.service!
					cout << "Receiving coin name..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();

					// Coin name
					UART::ustring ustrcname;
					UART::ustring *ustrcname_ptr = &ustrcname;
					uart_connection.uart_rxcommunicate(ustrcname_ptr);
					// Get contents in string format
					string coinname;
					string *coinname_ptr = &coinname;
					uart_connection.vect2str(coinname_ptr, ustrcname); // ustring -> string
					cout << "Coinname: " << coinname << endl;

					// Default RX bufsize
					uart_connection.defaultrx_buffer();

					string wallet_id; // Wallet id
					// Get wallet_id if we are not requesting ALL addresses
					if( coinname != "All" )
					{
						cout << "Receiving wallet id..." << endl;
						// Get expected buffer size for the next iteration
						uart_connection.changerx_buffer();

						// Wallet id
						UART::ustring ustrwid;
						UART::ustring *ustrwid_ptr = &ustrwid;
						uart_connection.uart_rxcommunicate(ustrwid_ptr);
						// Get contents in string format
						string *wallet_id_ptr = &wallet_id;
						uart_connection.vect2str(wallet_id_ptr, ustrwid); // ustring -> string
						cout << "Wallet id: " << wallet_id << endl;

						// Default bufsize
						uart_connection.defaultrx_buffer();
					}

					// Addresses
					string public_key;
					string public_address;
					string coin_idname;
					int returned_records = 0; // All records returned from db

					// ------------------------- public key -------------------- //
					// Get public key!
					string sqlqry = "";
					if( coinname == "All" )
						sqlqry  = "SELECT public_key FROM addresses ORDER BY ID ASC";
					else
						sqlqry  = "SELECT public_key FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname + "') AND wallet_id = " + wallet_id + " ORDER BY ID ASC";
					returned_records = hashbankdb.sql_query(sqlqry); // Records returned
					// Retrieve values from db
					if( returned_records > 0 )
					{
						cout << "Total records: " << returned_records << endl;
						// Get all records
						for( int i=0; i<returned_records; i++ )
						{
							// Default bufsize
							uart_connection.defaulttx_buffer();

							// Current public key
							public_key = hashbankdb.sql_vals[i];

							// Send public key through serial port
							UART::ustring nxt_bufsize;
							UART::ustring public_key_tx;

							// Buffer size of public key
							nxt_bufsize = uart_connection.changetx_buffer(public_key);
							// Convert string to ustring
							uart_connection.str2vect(&public_key_tx, public_key);
							// Send buffer
							cout << "sending next buf..." << endl;
							uart_connection.uart_txcommunicate(nxt_bufsize);
							cout << i << ") sending public key..." << endl;
							// Send actual contents
							uart_connection.uart_txcommunicate(public_key_tx);

						}
					}else{
						// Send bufsize 0, nothing found!
					}

					UART::ustring nxt_bufsize; 
					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);

					cout << "Sending addresses..." << endl;

					// Default RX/TX buffer size
					uart_connection.defaultrx_buffer();
					uart_connection.defaulttx_buffer();

					// ------------------------- public Address -------------------- //
					// Get public key!
					if( coinname == "All" )
						sqlqry  = "SELECT address FROM addresses ORDER BY ID ASC";
					else
						sqlqry  = "SELECT address FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname + "') AND wallet_id = " + wallet_id + " ORDER BY ID ASC";
					returned_records = hashbankdb.sql_query(sqlqry); // Records returned
					// Retrieve values from db
					if( returned_records > 0 )
					{
						cout << "Total records: " << returned_records << endl;
						// Get all records
						for( int i=0; i<returned_records; i++ )
						{
							// Default bufsize
							uart_connection.defaulttx_buffer();

							// Current public address
							public_address = hashbankdb.sql_vals[i];

							// Send public key through serial port
							UART::ustring nxt_bufsize;
							UART::ustring public_key_tx;

							// Buffer size of public key
							nxt_bufsize = uart_connection.changetx_buffer(public_address);
							// Convert string to ustring
							uart_connection.str2vect(&public_key_tx, public_address);
							// Send buffer
							cout << "sending next buf..." << endl;
							uart_connection.uart_txcommunicate(nxt_bufsize);
							cout << i << ") sending public address..." << endl;
							// Send actual contents
							uart_connection.uart_txcommunicate(public_key_tx);

						}
					}else{
						// Send bufsize 0, nothing found!
					}

		 
					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);


					// If all addresses were requested!
					if( coinname == "All" )
					{
					
						// COIN Ids (based on db)
						cout << "Sending coin ids..." << endl;

						// Default RX/TX buffer size
						uart_connection.defaultrx_buffer();
						uart_connection.defaulttx_buffer();

						// ------------------------- COIN ID -------------------- //
						// Get public key!
						sqlqry  = "SELECT coin_id FROM addresses ORDER BY ID ASC";
						returned_records = hashbankdb.sql_query(sqlqry); // Records returned
						// Retrieve values from db
						if( returned_records > 0 )
						{
							cout << "Total records: " << returned_records << endl;
							// Get all records
							for( int i=0; i<returned_records; i++ )
							{
								// Default bufsize
								uart_connection.defaulttx_buffer();

								// Current coin ID
								coin_idname = hashbankdb.sql_vals[i];

								// Send public key through serial port
								UART::ustring nxt_bufsize;
								UART::ustring coin_id_tx;

								// Buffer size of public key
								nxt_bufsize = uart_connection.changetx_buffer(coin_idname);
								// Convert string to ustring
								uart_connection.str2vect(&coin_id_tx, coin_idname);
								// Send buffer
								cout << "sending next buf..." << endl;
								uart_connection.uart_txcommunicate(nxt_bufsize);
								cout << i << ") sending coin id..." << endl;
								// Send actual contents
								uart_connection.uart_txcommunicate(coin_id_tx);

							}
						}else{
							// Send bufsize 0, nothing found!
						}

						// Default bufsize
						uart_connection.defaulttx_buffer();
						// Send bufsize 0f 0
						nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
						// Convert string to ustring
						uart_connection.uart_txcommunicate(nxt_bufsize);

						cout << "Sending wallet ids..." << endl;

						// Default RX/TX buffer size
						uart_connection.defaultrx_buffer();
						uart_connection.defaulttx_buffer();

						// ------------------------- Wallet id -------------------- //
						sqlqry  = "SELECT wallet_id FROM addresses ORDER BY ID ASC";
						// else // --> Not else needed here; Wallet_id remains constant accross all addresses
						//sqlqry  = "SELECT wallet_id FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname + "') AND wallet_id = " + wallet_id + " ORDER BY ID ASC";
						returned_records = hashbankdb.sql_query(sqlqry); // Records returned
						// Retrieve values from db
						if( returned_records > 0 )
						{
							cout << "Total records: " << returned_records << endl;
							// Get all records
							for( int i=0; i<returned_records; i++ )
							{
								// Default bufsize
								uart_connection.defaulttx_buffer();

								// Current public address
								public_address = hashbankdb.sql_vals[i];

								// Send public key through serial port
								UART::ustring nxt_bufsize;
								UART::ustring public_key_tx;

								// Buffer size of public key
								nxt_bufsize = uart_connection.changetx_buffer(public_address);
								// Convert string to ustring
								uart_connection.str2vect(&public_key_tx, public_address);
								// Send buffer
								cout << "sending next buf..." << endl;
								uart_connection.uart_txcommunicate(nxt_bufsize);
								cout << i << ") sending wallet id..." << endl;
								// Send actual contents
								uart_connection.uart_txcommunicate(public_key_tx);

							}
						}else{
							// Send bufsize 0, nothing found!
						}

		 
						// Default bufsize
						uart_connection.defaulttx_buffer();
						// Send bufsize 0f 0
						nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
						// Convert string to ustring
						uart_connection.uart_txcommunicate(nxt_bufsize);

						cout << "Sending wallet names..." << endl;

						// Default RX/TX buffer size
						uart_connection.defaultrx_buffer();
						uart_connection.defaulttx_buffer();

						// ------------------------- Wallet id -------------------- //
						sqlqry  = "SELECT ID,wallet_name,wallet_type FROM wallets ORDER BY ID ASC";
						// else // --> Not else needed here; Wallet_id remains constant accross all addresses
						//sqlqry  = "SELECT wallet_id FROM addresses WHERE coin_id = (SELECT coin_id FROM coin WHERE coin_name = '" + coinname + "') AND wallet_id = " + wallet_id + " ORDER BY ID ASC";
						returned_records = hashbankdb.sql_query(sqlqry); // Records returned
						// Retrieve values from db
						if( returned_records > 0 )
						{
							cout << "Total records: " << returned_records << endl;
							// Get all records
							for( int i=0; i<returned_records; i++ )
							{
								// Default bufsize
								uart_connection.defaulttx_buffer();

								// Current public address
								public_address = hashbankdb.sql_vals[i];

								// Send public key through serial port
								UART::ustring nxt_bufsize;
								UART::ustring public_key_tx;

								// Buffer size of public key
								nxt_bufsize = uart_connection.changetx_buffer(public_address);
								// Convert string to ustring
								uart_connection.str2vect(&public_key_tx, public_address);
								// Send buffer
								cout << "sending next buf..." << endl;
								uart_connection.uart_txcommunicate(nxt_bufsize);
								cout << i << ") sending wallet id/name/wallet_type..." << endl;
								// Send actual contents
								uart_connection.uart_txcommunicate(public_key_tx);

							}
						}else{
							// Send bufsize 0, nothing found!
						}

		 
						// Default bufsize
						uart_connection.defaulttx_buffer();
						// Send bufsize 0f 0
						nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
						// Convert string to ustring
						uart_connection.uart_txcommunicate(nxt_bufsize);

					}
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!
					break;
				}

				case uart_connection.PC_CRTWLT:
				{
					//send_usbpage_lcd(4, &uart_connection); // Send page number to lcd.service!
					cout << "Receiving wallet name..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();

					// Coin name
					UART::ustring ustrwname;
					UART::ustring *ustrwname_ptr = &ustrwname;
					uart_connection.uart_rxcommunicate(ustrwname_ptr);
					// Get contents in string format
					string coinname;
					string *coinname_ptr = &coinname;
					uart_connection.vect2str(coinname_ptr, ustrwname); // ustring -> string
					cout << "Wallet Name: " << coinname << endl;

					// Default RX bufsize
					uart_connection.defaultrx_buffer();

					cout << "Receiving wallet type..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();

					// Wallet type
					UART::ustring ustrwtype;
					UART::ustring *ustrwtype_ptr = &ustrwtype;
					uart_connection.uart_rxcommunicate(ustrwtype_ptr);
					// Get contents in string format
					string wallet_type;
					string *wallet_type_ptr = &wallet_type;
					uart_connection.vect2str(wallet_type_ptr, ustrwtype); // ustring -> string
					cout << "Wallet Type: " << wallet_type << endl;

					// Default bufsize
					uart_connection.defaultrx_buffer();

					// Run script to create new wallet
					string script_cmd = "nodejs ../cryptolibs/create_wallet.js " + coinname + " " + wallet_type;
					char * COIN_CMD = new char[script_cmd.length()+1];
					strcpy (COIN_CMD, script_cmd.c_str());
					cout << "running script: " << COIN_CMD << endl;
					int script_status = system(COIN_CMD); // Run script! if returned 0 -- sucess

					UART::ustring nxt_bufsize;
					UART::ustring path_id_tx;
					// Script ran succesfully
					if( !script_status )
					{
						string sqlqry  = "SELECT ID FROM wallets ORDER BY ID DESC LIMIT 1;";
						int returned_records = hashbankdb.sql_query(sqlqry); // Records returned

						string path_val; // Path value
						// Retrieve values from db
						if( returned_records > 0 )
						{
							path_val = hashbankdb.sql_vals[0];

							// Buffer size of path value
							nxt_bufsize = uart_connection.changetx_buffer(path_val); 
							// Convert string to ustring
							uart_connection.str2vect(&path_id_tx, path_val);
							// Send buffer
							cout << "sending nxt_bufsize..." << endl;
							uart_connection.uart_txcommunicate(nxt_bufsize);
							cout << "sending path id..." << endl;
							// Send actual contents
							uart_connection.uart_txcommunicate(path_id_tx);
						}
					}
					else{
						// ERROR! Could not create new wallet!

					}

					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!

					break;
				}


				case uart_connection.PC_SGNTRN:
				{
					//send_usbpage_lcd(5, &uart_connection); // Send page number to lcd.service!
					cout << "Receiving transaction details..." << endl;

					// WALLET ID
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();
					// Coin name
					UART::ustring ustrwid;
					UART::ustring *ustrwid_ptr = &ustrwid;
					uart_connection.uart_rxcommunicate(ustrwid_ptr);
					// Get contents in string format
					string wallet_id;
					string *wallet_id_ptr = &wallet_id;
					uart_connection.vect2str(wallet_id_ptr, ustrwid); // ustring -> string

					// COIN NAME
					// Get expected buffer size for the next iteration
					uart_connection.defaultrx_buffer(); 
					uart_connection.changerx_buffer();
					// Coin name
					UART::ustring ustrcname;
					UART::ustring *ustrcname_ptr = &ustrcname;
					uart_connection.uart_rxcommunicate(ustrcname_ptr);
					// Get contents in string format
					string coinname;
					string *coinname_ptr = &coinname;
					uart_connection.vect2str(coinname_ptr, ustrcname); // ustring -> string

					// AMOUNT
					// Get expected buffer size for the next iteration
					uart_connection.defaultrx_buffer(); 
					uart_connection.changerx_buffer();
					// Amount
					UART::ustring ustramount;
					UART::ustring *ustramount_ptr = &ustramount;
					uart_connection.uart_rxcommunicate(ustramount_ptr);
					// Get contents in string format
					string amount;
					string *amount_ptr = &amount;
					uart_connection.vect2str(amount_ptr, ustramount); // ustring -> string

					// ADDRESS TO
					// Get expected buffer size for the next iteration
					uart_connection.defaultrx_buffer();
					uart_connection.changerx_buffer();
					// Address to
					UART::ustring ustraddrto;
					UART::ustring *ustraddrto_ptr = &ustraddrto;
					uart_connection.uart_rxcommunicate(ustraddrto_ptr);
					// Get contents in string format
					string address_to;
					string *address_to_ptr = &address_to;
					uart_connection.vect2str(address_to_ptr, ustraddrto); // ustring -> string
					cout << "Sending " << amount << " " << coinname << " to " << address_to << " from wallet_id " << wallet_id << "..." << endl;	

					// Create new FIFO (named pipe)
					IPC named_pipew("sgntrnw");
					IPC named_piper("sgntrnr");

					UART::ustring JSON_data; // JSON data received 
					// Get all relevant information to sign the transaction (e.g Unspent Outputs)
					while( uart_connection.get_rxbufsize() != 0 )
					{
						// Set buffer to default again
						uart_connection.defaultrx_buffer(); 
						// Get expected NEW buffer size for the next iteration
						uart_connection.changerx_buffer();

						if( uart_connection.get_rxbufsize() == 0) // Next buffer is zero, so we finished reading file
							break;

						// Read data
						UART::ustring ustrfbuffer;
						UART::ustring *ustrfbuffer_ptr = &ustrfbuffer;
						uart_connection.uart_rxcommunicate(ustrfbuffer_ptr);

						// Store in RAM all the JSON data to send it all in one big chunk to the named pipe (data shouldn't be too big)
						for( int i=0; i<ustrfbuffer.size(); i++ )
							JSON_data.push_back(ustrfbuffer[i]);

					}

					// Process cmd
					string command_node = "nodejs ../cryptolibs/" + coinname + "_transactionHB.js " + "'" + named_pipew.get_pipepath() + "' " + "'" + named_piper.get_pipepath() + "' " + wallet_id +  " &"; 
					//cout << command_node << endl;
					char * CMD = new char[command_node.length()+1];
					strcpy (CMD, command_node.c_str());
					// Open process
					system(CMD);

					// Open pipe
					named_pipew.open_fifo('w');
					// write to FIFO (named pipe)
					named_pipew.write_fifo(JSON_data);
					// Close FIFO
					named_pipew.close_fifo();

					
					// Open pipe
					UART::ustring tx_signedhex;
					named_piper.open_fifo('r');
					// write to FIFO (named pipe)
					named_piper.read_fifo(&tx_signedhex);
					// Close FIFO
					named_piper.close_fifo();

					//cout << "Received: " << endl;
					//named_piper.print_ustring(tx_signedhex);
					cout << "Transaction signed! " << endl;
					// Set buffer to default again
					uart_connection.defaultrx_buffer(); 

					/* SEND to UART */
					int hexsize = tx_signedhex.size(); // Number of bytes to send
					int pages_num = int(ceil(hexsize/double(uart_connection.get_maxbuffer()))); // Determine how many iterations are needed to send the signed hex number .. (Count the 0th iteration)
					int iter_beg = 0;
					int iter_end = uart_connection.get_maxbuffer();

					cout << "pages_num: " << pages_num << endl;
					UART::ustring nxt_bufsize; // Next buffer size
					// Send in multiple chunks
					for( int i=0; i<pages_num; i++ )
					{
						// Default bufsize
						uart_connection.defaulttx_buffer();

						UART::ustring bytes_tx; // bytes to sends
						// Send the file chunk by chunk
						if (pages_num <= 1)
							bytes_tx = tx_signedhex;
						else
						{
							for( int j=iter_beg; j<iter_end; j++ )
								bytes_tx.push_back(tx_signedhex[j]); 
							if (i < pages_num-2) // If not second to last page
							{
								// Update iterators
								iter_beg = iter_end;
								iter_end += uart_connection.get_maxbuffer();
							}
							else // last page
							{
								iter_beg = iter_end;
								iter_end = hexsize;
							}
						}

						// Get contents in string format to count number of bytes and send next buffer size
						string bytes_send;
						uart_connection.vect2str(&bytes_send, bytes_tx); // ustring -> string
						// Send next bufsize 
						nxt_bufsize = uart_connection.changetx_buffer(bytes_send); 
						// Convert string to ustring
						uart_connection.uart_txcommunicate(nxt_bufsize);
						// Send actual contents!
						uart_connection.uart_txcommunicate(bytes_tx);
					}

					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);

					delete[] CMD;
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!

					break;
				}

				// Read wallet to NFC
				case uart_connection.PC_NFCRAD:
				{
					//send_usbpage_lcd(6, &uart_connection); // Send page number to lcd.service!
					UART::ustring nfc_cmd; // ustring to send to pipe
					string command = to_string(uart_connection.NFC_READ); // NFC command
					// Convert string to ustring
					uart_connection.str2vect(&nfc_cmd, command);
					// Access NFC process via pipe
					IPC named_pipecmd("NFCcommands");
					// Open pipe
					named_pipecmd.open_fifo('w');
					// write to FIFO (named pipe)
					named_pipecmd.write_fifo(nfc_cmd);
					named_pipecmd.close_fifo();

					// Open pipe
					UART::ustring nfc_contents;
					IPC pipenfc_contents("NFCcontents");
					pipenfc_contents.open_fifo('r');
					pipenfc_contents.read_fifo(&nfc_contents);
					pipenfc_contents.close_fifo();
					// Str nfc_contents
					string nfc_strcontents;
					uart_connection.vect2str(&nfc_strcontents, nfc_contents); // ustring -> string

					// Send info to UART
					UART::ustring nxt_bufsize; // Next buffer size
					nxt_bufsize = uart_connection.changetx_buffer(nfc_strcontents);
					// Send buffer
					cout << "sending next buf..." << endl;
					uart_connection.uart_txcommunicate(nxt_bufsize);
					cout << "sending NFC contents..." << endl;
					// Send actual contents
					uart_connection.uart_txcommunicate(nfc_contents);

					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!

					break;
				}

				// Read NFC wallet 
				case uart_connection.PC_NFCWRT:
				{
					//send_usbpage_lcd(7, &uart_connection); // Send page number to lcd.service!
					cout << "Receiving wallet id..." << endl;
					// Get expected buffer size for the next iteration
					uart_connection.changerx_buffer();
					// Wallet id
					UART::ustring walletid;
					UART::ustring *walletid_ptr = &walletid;
					uart_connection.uart_rxcommunicate(walletid_ptr);
					// Get contents in string format
					string wallet_id;
					string *wallet_id_ptr = &wallet_id;
					uart_connection.vect2str(wallet_id_ptr, walletid); // ustring -> string
					cout << "Wallet ID: " << wallet_id << endl;

					// Default bufsize
					uart_connection.defaultrx_buffer();

					// Send command to NFC process
					UART::ustring nfc_cmd; // ustring to send to pipe
					string command = to_string(uart_connection.NFC_WRITE); // NFC command
					uart_connection.str2vect(&nfc_cmd, command);
					IPC named_pipecmd("NFCcommands");
					named_pipecmd.open_fifo('w');
					named_pipecmd.write_fifo(nfc_cmd);
					named_pipecmd.close_fifo(); // Close pipe

					string sqlqry  = "SELECT mnemonic,wallet_type FROM wallets WHERE ID = " + wallet_id + ";";
					//cout << "sqlqry: " << sqlqry << endl;

					int returned_records = hashbankdb.sql_query(sqlqry); // Records returned

					//cout << "returned_records " << returned_records << endl;
					string mnemonic; // mnemonic
					string wallet_type; // Wallet type
					// Retrieve values from db
					if( returned_records > 0 )
					{
						//cout << "Sending to NFC process... " << endl;
						mnemonic = hashbankdb.sql_vals[0];
						wallet_type = hashbankdb.sql_vals[1];

						// Is this a backup wallet? Or a NFC wallet? -- see wallet_types in db
						mnemonic += " :" + wallet_type; // Add wallet_type at the end

						// convert to ustrin
						UART::ustring mnemonic_wallet;
						uart_connection.str2vect(&mnemonic_wallet, mnemonic);

						// ustring to send to pipe
						IPC named_pipewr("NFCwallet");
						named_pipewr.open_fifo('w');
						named_pipewr.write_fifo(mnemonic_wallet);
						named_pipewr.close_fifo(); // Close pipe

						// Confirm contents of NFC card!
						// Open pipe
						UART::ustring nfc_contents;
						IPC pipenfc_contents("NFCcontents");
						pipenfc_contents.open_fifo('r');
						pipenfc_contents.read_fifo(&nfc_contents);
						pipenfc_contents.close_fifo();
						// Str nfc_contents
						string nfc_strcontents;
						uart_connection.vect2str(&nfc_strcontents, nfc_contents); // ustring -> string

						// Send info to UART
						UART::ustring nxt_bufsize; // Next buffer size
						nxt_bufsize = uart_connection.changetx_buffer(nfc_strcontents);
						// Send buffer
						cout << "sending next buf..." << endl;
						uart_connection.uart_txcommunicate(nxt_bufsize);
						cout << "sending NFC contents..." << endl;
						// Send actual contents
						uart_connection.uart_txcommunicate(nfc_contents);

						// Default bufsize
						uart_connection.defaulttx_buffer();
						// Send bufsize 0f 0
						nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
						// Convert string to ustring
						uart_connection.uart_txcommunicate(nxt_bufsize);


					}
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!

					break;
				}

				// execute update script
				case uart_connection.PC_UPDATE:
				{
					//send_usbpage_lcd(8, &uart_connection); // Send page number to lcd.service!
					// Run script to update files
					string script_cmd = ".././update_script";
					char * UPDATE_CMD = new char[script_cmd.length()+1];
					strcpy (UPDATE_CMD, script_cmd.c_str());
					int script_status = system(UPDATE_CMD); // Run script! if returned 0 -- sucess
					// Free memory
					delete[] UPDATE_CMD;

					UART::ustring nxt_bufsize; // buffer size

					// Default bufsize
					uart_connection.defaulttx_buffer();
					// Send bufsize 0f 0
					nxt_bufsize = uart_connection.changetx_buffer(""); // nothing more to send (all public keys sent succesfully!)
					// Convert string to ustring
					uart_connection.uart_txcommunicate(nxt_bufsize);
					//send_usbpage_lcd(0, &uart_connection); // Send page number to lcd.service!

					break;
				}


				// Command not recognized, request data again?
				//default:
				//	request_repeatdata(); // Request data to be resent again 
				//break;
			}

		}

	}




	return 0;
} 