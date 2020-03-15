#include <string.h>
#include <fcntl.h>
#include "PWM.h"

// Defines constructor, Sets directories.
// Ports available: 0,2,4. Channels available: 0,1. (6 total).
PWM::PWM( std::string port, std::string ch):Port(port),Channel(ch){
	
	// Checks if the channel is already initiated, looks for created directory.
	std::string dir = "/sys/class/pwm/pwmchip"+Port+"/pwm-"+Port+":"+Channel+"/period";
	char * Period_dir = new char [dir.length()+1];		// Casting literal string to char*
	std::strcpy (Period_dir, dir.c_str());
	int PWM_ = open(Period_dir, O_RDONLY);			// Tries to open file, if it exists.
	if(PWM_ == -1)						// If it doesn't, create directories for selected channel.
	{
		// Export creates directories for chosen port and channel
		std::string Export = "/sys/class/pwm/pwmchip"+Port+"/export"; // Export directory for selected port
		char * Export_dir = new char [Export.length()+1];	// Casting of literal string to char*
		std::strcpy (Export_dir, Export.c_str());
		char * CH = new char[Channel.length()+1];		// Creates Char* to choose channel. 0 and 1 available
		std::strcpy (CH, Channel.c_str());
		
		int PWM_ = open(Export_dir,O_WRONLY);
		if(PWM_ != -1)
		{
			write(PWM_, CH, sizeof(char));			// Writes value to choose channel.
			close(PWM_);
		}
		delete[] Export_dir;
		delete[] CH;
	}
	else							// If it exists, ignores above.
	{
		close(PWM_);
	}
	delete[] Period_dir;					// Clears pointers
}
void PWM::run(std::string p, std::string dc){			//Values in nanoseconds.
	
	Period =p;
	Duty_Cycle = dc;
	std::string dir = "/sys/class/pwm/pwmchip"+Port+"/pwm-"+Port+":"+Channel+"/period"; 	//Period directory.
	char * Period_dir = new char [dir.length()+1];						// Casting address to char*
	std::strcpy (Period_dir, dir.c_str());		
	int PWM_ = open(Period_dir,O_WRONLY);
	char * P = new char [Period.length()+1];
	std::strcpy (P, Period.c_str());
	while(PWM_ == -1)
	{
		PWM_ = open(Period_dir,O_WRONLY);		//Only passes if file is correctly opened.
	}
	if(PWM_ != -1)
	{
		write(PWM_, P, Period.length());		// writes Period value
		close(PWM_);					// and closes file.
	}
	delete[] Period_dir;					//Clears pointers.
	delete[] P;
	
	dir = "/sys/class/pwm/pwmchip"+Port+"/pwm-"+Port+":"+Channel+"/enable";			//Enable directory
	char * En_dir = new char [dir.length()+1];						//Casting address to char*
	std::strcpy (En_dir, dir.c_str());
	PWM_ = open(En_dir,O_WRONLY);
	if(PWM_ != -1)
	{
		write(PWM_,"1", sizeof(char));			//0 disabled, 1 enabled.
		close(PWM_);
	}
	delete[] En_dir;
	
		
	dir = "/sys/class/pwm/pwmchip"+Port+"/pwm-"+Port+":"+Channel+"/duty_cycle";		// Duty cycle directory
	char * DC_dir = new char [dir.length()+1];					// Casting of address to char*
	std::strcpy (DC_dir, dir.c_str());
	char * DC = new char [Duty_Cycle.length()+1];
	std::strcpy (DC, Duty_Cycle.c_str());
	PWM_ = open(DC_dir,O_WRONLY);
	if(PWM_ != -1)
	{
		write(PWM_, DC, Duty_Cycle.length());				// writes value and closes. Duty_Cycle < Period.
		close(PWM_);							// Else it doesn't change its value. 
	}
	delete[] DC_dir;
	delete[] DC;
}
