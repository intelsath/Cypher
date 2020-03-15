#ifndef _PWM_H_
#define _PWM_H_

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <cstring>

// PWM driver directories


using namespace std;


class PWM {
	private:
		std::string Period;						//PWM period, nanoseconds
		std::string Duty_Cycle;						//PWM Duty cycle, nanoseconds
		std::string Port;						//PWM Port (3 available)
		std::string Channel;						//PWM Channel (2 for every port)
	public:
		PWM(std::string, std::string);					//Constructor, creates the directories.
		void run(std::string, std::string);				//Receives Period and Duty Cycle
	};
#endif
