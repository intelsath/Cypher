// GPIO controller

extern "C" {
	#include <stdio.h> // standard input / output functions
	#include <fcntl.h> // File control definitions
	#include <errno.h> // Error number definitions
	#include <unistd.h> // UNIX standard function definitions
}

#include <iostream>
#include <cstring>
#include "GPIO.h"

// Initializes pin number
GPIOPIN::GPIOPIN(std::string pin_num, bool detect_edge){
	this->pin_num = pin_num; // Set pin number to control

	// Detect rising and falling edge? i.e interrupts
	if( detect_edge )
	{
		int status = enable_interrupt();

		if( status )
			std::cout << "Interrupts enable: Rising/Falling edge detection" << std::endl;
		else
			std::cout << "Error: Could not enable interrupts!" << std::endl;
		
	}
}

// Read analogue GPIO
int GPIOPIN::read_agpio(std::string file, int * output)
{
	// Filename
	char * cstr_file = new char [file.length()+1];
	std::strcpy (cstr_file, file.c_str()); // Copy string to cstr

	// Open file
	int fd = open(cstr_file,O_RDONLY);
	int status = 0;
	if(fd == -1) // if open is unsucessful
	{
		status = 0;
	}
	else
	{
		status = 1; // success
	}

	if( status )
	{
		// Read
		const int bufsize = 16;
		char cstr_data[bufsize];
		int n = read(fd, &cstr_data, bufsize);

		/* Error Handling */
		if (n < 0)
		{
			 //std::cout << "Error reading: " << std::strerror(errno) << std::endl;
			 status = 0;
		}else{
			int outbuf = atoi(cstr_data);
			*output = outbuf;
			// std::cout << "Read data: " << cstr_data << std::endl;
		}

		close(fd);		// close file
	}

	delete[] cstr_file; // free memory

	return status;

}

int GPIOPIN::write_gpio(std::string file, std::string data){

	char * cstr_file = new char [file.length()+1];
	std::strcpy (cstr_file, file.c_str()); // Copy string to cstr
	// Open GPIO direction file
	int fd = open(cstr_file, O_WRONLY);
	int status = 0;
	if(fd == -1) // if open is unsucessful
	{
		//printf("Unable to open GPIO file\n");
		status = 0;
	}
	else
	{
		//printf("GPIO ready to write\n");
		status = 1; // success
	}

	if( status )
	{
		int n = 0; // Write status
		char * cstr_data = new char [data.length()+1];
		std::strcpy (cstr_data, data.c_str()); // Copy string to cstr

		// write
		n = write(fd, cstr_data, data.length());

		/* Error Handling */
		if (n < 0)
		{
			 //std::cout << "Error reading: " << std::strerror(errno) << std::endl;
			 status = 0;
		}else{
			//std::cout << "Succesfully changed " << file << " file with data: " << data << std::endl;
		}

		delete[] cstr_data; // free memory
		close(fd);		// close file
	}


	delete[] cstr_file; // free memory
	

	return status;
}

int GPIOPIN::pin_io(int direction){
	
	std::string dirfd = "/sys/class/gpio/gpio" + pin_num + "/direction"; 

	int status;
	// Write Direction
	switch(direction){
		case PINOUT:
			status = write_gpio(dirfd, "out");
		break;
		case PININ:
			status = write_gpio(dirfd, "in");
		break;
	}


	return status;
}

int GPIOPIN::set_pin(int pin_status){
	std::string dirfd = "/sys/class/gpio/gpio" + pin_num + "/value"; 

	int status;
	// Write Direction
	switch(pin_status){
		case PINLOW:
			status = write_gpio(dirfd, "0");
		break;
		case PINHIGH:
			status = write_gpio(dirfd, "1");
		break;
	}


	return status;
}

int GPIOPIN::enable_interrupt(){
	std::string dirfd = "/sys/class/gpio/gpio" + pin_num + "/edge"; 

	int status;
	// Write to edge (to enable interrupts)
	status = write_gpio(dirfd, "both"); // Detect both rising and decreasing edge


	return status;
}

int GPIOPIN::detect_edge()
{
	// Get file
	std::string dirfd = "/sys/class/gpio/gpio" + pin_num + "/value"; 
	char * cstr_file = new char [dirfd.length()+1];
	std::strcpy (cstr_file, dirfd.c_str()); // Copy string to cstr

	//std::cout << "Detecting a change through polling..." << std::endl;
	// polling
	int fd = open(cstr_file, O_RDONLY);
	struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLPRI;
    pfd.revents = 0;

	// Read
	int output = 0;
	read(fd, &output, 1);

	// Is it ready yet? Has any change been detected?
	int ready = poll(&pfd, 1, -1);
	//std::cout << "Done-> ready: " << ready << std::endl;

	delete[] cstr_file;

	return output;
}

void GPIOPIN::update_adc()
{
	std::string adc_io = "/sys/bus/iio/devices/iio:device0/" + pin_num;

	// Read ADC values
	int ADC;
	int * ADC_OUTPUT = &ADC;
	read_agpio(adc_io, ADC_OUTPUT); // Read analogue gpio

	this->ADC = ADC;
	// std::cout << "ADC: " << this->ADC << std::endl;
}