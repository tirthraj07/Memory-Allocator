#include <iostream>
#include "allocator.h"
#include "garbage_collector.h"

int main() {

	Allocator& alloc = Allocator::getInstance();		        // Get the instance of allocator 
	// GC is enabled by default

	int* int_arr = (int*)alloc.allocate(sizeof(int) * 10, (void**)&int_arr);		// Allocate 10 integers in heap

	/*
	Here we provide an option root parameter.
	root (Optional) A pointer to a pointer where the address of the
				 allocated memory will be stored. If provided, the allocated
				 memory is treated as a GC root, and its reference is tracked
				 by the garbage collector to prevent it from being reclaimed.
				 Use this when you need persistent references in your program.

	*/

	for (int i = 0; i < 10; i++) {
		int_arr[i] = i;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << int_arr[i] << " ";
	}
	std::cout << std::endl;

	alloc.deallocate(int_arr);

}