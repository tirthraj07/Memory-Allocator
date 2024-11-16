#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H
#pragma once
#include "chunk_metadata.h"
#include <sstream>
#include <string>
#include <iostream>

class Garbage_Collector {
private:
	std::ostringstream out;											///< Output stream for logging purposes.
	Garbage_Collector(bool debug_mode, void* heap_start, size_t HEAP_CAPACITY);
	void log_info();

	bool DEBUG_MODE;
	void** potential_stack_vars_containing_roots_list[1000];
	int potential_roots_size = 0;

	void* root_chunk_list[1000];
	int root_chunk_list_size = 0;

	void* heap_start;
	std::size_t HEAP_CAPACITY;

	bool is_pointer_within_heap(void* ptr);
	void get_roots();
	void unmark_chunks();
	void find_chunks_within_chunk(Chunk_Metadata* top);
	void sweep_phase();

public:
	
	static Garbage_Collector& getInstance(void* heap_start, size_t HEAP_CAPACITY, bool debug_mode = false);
	
	Garbage_Collector(const Garbage_Collector&) = delete;
	Garbage_Collector& operator=(const Garbage_Collector&) = delete;

	void gc_collect();
	void add_gc_roots(void** root);
	void gc_dump();
	void mark_phase();

	friend class Allocator;
};


#endif