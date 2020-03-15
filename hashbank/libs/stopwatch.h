#ifndef _STOPWATCH_H
#define _STOPWATCH_H

#include <ctime>
#include <ratio>
#include <chrono>

using namespace std::chrono;

class STOPWATCH{
	public:
		STOPWATCH();
		void start(); // Start timer
		void reset(); // Reset timer

		double get_epoch();
		
	private:
		high_resolution_clock::time_point t1;
		high_resolution_clock::time_point t2;
		double epoch;

		bool stopped;
		bool init;
};


#endif