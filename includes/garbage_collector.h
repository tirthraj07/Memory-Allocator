#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H
#pragma once

#include <sstream>
#include <string>
#include <iostream>

class Garbage_Collector {
private:
	std::ostringstream out;											///< Output stream for logging purposes.
	Garbage_Collector(bool debug_mode);
	void log_info();
	bool DEBUG_MODE;


public:
	
	static Garbage_Collector& getInstance(bool debug_mode = false);
	
	void gc_collect();


};


#endif