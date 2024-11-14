#include "garbage_collector.h"
#define LBR '\n'

#include <sstream>
#include <string>
#include <iostream>

Garbage_Collector::Garbage_Collector(bool debug_mode) {
    this->DEBUG_MODE = debug_mode;
    out << "Garbage Collector Instantiated" << LBR;
    log_info();
}

void Garbage_Collector::log_info(){
    if (DEBUG_MODE) {
        std::string str = out.str();
        std::cout << "[INFO]    " << str << LBR;
    }
    out.str(""); // Clear out the contents after logging
    out.clear();
}

Garbage_Collector& Garbage_Collector::getInstance(bool debug_mode)
{
    static Garbage_Collector gc(debug_mode);
    return gc;
}

void Garbage_Collector::gc_collect()
{
    out << "Called GC Collect" << LBR;
    log_info();
}
