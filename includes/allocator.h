#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef>
#include <unistd.h>
#include "chunk_metadata.h"

class Allocator{
private:
	static const std::size_t INITIAL_HEAP_CAPACITY = 1024 * 1024; 	// 1 MB initial heap
	void* heap_start;
	std::size_t used_heap_size;
	std::size_t HEAP_CAPACITY;

	int expand_heap(std::size_t size);
	
public:
	Allocator();
	void* allocate(std::size_t size);
	void deallocate(void* ptr);
	void heap_dump();
};

#endif 