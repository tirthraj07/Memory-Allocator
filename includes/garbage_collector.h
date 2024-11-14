#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H
#pragma once

class Garbage_Collector {
private:
	Garbage_Collector gc;

	Garbage_Collector();
	void log_info(const std::string& str);

public:
	
	static Garbage_Collector getInstance();
	


};


#endif