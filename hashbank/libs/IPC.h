// Inter-Process Communication named-pipe

#ifndef _IPC_H
#define _IPC_H

#include <string>
// Some functions inherited from FHANDLER
#include "fhandler.h"

class IPC : public FHANDLER{
	public:
		IPC();
		IPC(std::string);
		~IPC();
		
		// Pipe methods
		int create_fifo(); // Create named-pipe and open it 
		void open_fifo(char);
		void close_fifo();
		void delete_fifo(); // Delete FIFO from system
		int write_fifo(ustring); // Write to named-pipe
		int read_fifo(ustring*); // Read named-pipe

		std::string get_pipepath(){ return pipe_path; }


		
	private:
		char open_mode; // 'r' = O_RDONLY or 'w' = O_WRONLY
		std::string pipe_name;
		std::string pipe_path; // e.g /tmp/myfifo
};


#endif