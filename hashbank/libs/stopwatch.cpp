// Button class -- Inherited from GFX, image object
// Button with behavior when being touched

#include <iostream>
#include "stopwatch.h"

STOPWATCH::STOPWATCH()
{
	epoch = 0;
	// Is the timer stopped?
	init = false; // Clock not initialized yet
}

void STOPWATCH::start()
{
	stopped = false;
	if (!init) // t1 has not been ionitialized
		t1 = high_resolution_clock::now();
		//t1 = time(0);
	init = true; // initialized
}


void STOPWATCH::reset()
{
	// t1 = time(0);
	t1 = high_resolution_clock::now();
}

double STOPWATCH::get_epoch()
{
	// t2 = time(0);
	t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

	epoch = time_span.count();

	return epoch;
}

