#include <iostream>
#include "allocator.h"
#include "garbage_collector.h"

int main(){
	bool DEBUG_MODE = true;
	Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
	Garbage_Collector& gc = alloc.getGC();

	int* arr = (int*)alloc.allocate(10 * sizeof(int), (void**) &arr);
	std::cout << "&arr = " << &arr << std::endl;
	std::cout << "arr = " << arr << std::endl;

	gc.gc_collect();
	gc.gc_dump();

	int* arr2 = (int*)alloc.allocate(10 * sizeof(int), (void**)&arr2);
	std::cout << "&arr2 = " << &arr2 << std::endl;
	std::cout << "arr2 = " << arr2 << std::endl;

	gc.gc_collect();
	gc.gc_dump();


}