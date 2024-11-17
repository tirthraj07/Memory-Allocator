#include <iostream>
#include "allocator.h"
#include "garbage_collector.h"

int main() {
	bool DEBUG_MODE = true;
	Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
	Garbage_Collector& gc = alloc.getGC();

	int* arr = (int*)alloc.allocate(10 * sizeof(int), (void**)&arr);

	std::cout << "&arr = " << &arr << std::endl;
	std::cout << "arr = " << arr << std::endl;

	for (int i = 0; i < 10; i++) {
		arr[i] = i;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << arr[i] << " ";
	}												// 0 1 2 3 4 5 6 7 8 9

	std::cout << std::endl;

	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

	int** arr2 = (int**)alloc.allocate(3 * sizeof(int*), (void**)&arr2);
	std::cout << "&arr2 = " << &arr2 << std::endl;
	std::cout << "arr2 = " << arr2 << std::endl;

	for (int i = 0; i < 3; i++) {
		arr2[i] = (int*)alloc.allocate(10 * sizeof(int));
		for (int j = 0; j < 10; j++) {
			arr2[i][j] = (i + 1) * j;
		}
	}

	std::cout << "&arr2 = " << &arr2 << std::endl;			// 0x7ffc26e13ba0
	std::cout << "arr2 = " << arr2 << std::endl;			// 0x5603f55b2478

	for (int i = 0; i < 3; i++) {
		std::cout << arr2[i] << " : " << std::endl << "\t";
		for (int j = 0; j < 10; j++) {
			std::cout << arr2[i][j] << ", ";
		}
		std::cout << std::endl;
	}

	alloc.heap_dump();
	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

	/*
				--- SNIPPET OF DEBUG LOGS ---
	Chunks:
		Chunk at: 0x5603f55b2400, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2450, Prev: 0
		Chunk at: 0x5603f55b2450, Size: 24 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5603f55b2490, Prev: 0x5603f55b2400
		Chunk at: 0x5603f55b2490, Size: 40 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5603f55b24e0, Prev: 0x5603f55b2450
		Chunk at: 0x5603f55b24e0, Size: 40 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5603f55b2530, Prev: 0x5603f55b2490
		Chunk at: 0x5603f55b2530, Size: 40 bytes, Allocated, gc_mark : UNMARKED, Next: 0, Prev: 0x5603f55b24e0
	*/



	// assign function ensures that that the elements are not picked by gc during sweeping phase
	// it marks the area 'reachable'

	int* a = alloc.assign(&a, arr2[0]);
	int* b = alloc.assign(&b, arr2[1]);
	int* c = alloc.assign(&c, arr2[2]);

	/*
			--- SNIPPET OF DEBUG LOGS ---[
	Chunks:
		Chunk at: 0x5603f55b2400, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2450, Prev: 0
		Chunk at: 0x5603f55b2450, Size: 24 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2490, Prev: 0x5603f55b2400
		Chunk at: 0x5603f55b2490, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b24e0, Prev: 0x5603f55b2450
		Chunk at: 0x5603f55b24e0, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2530, Prev: 0x5603f55b2490
		Chunk at: 0x5603f55b2530, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0, Prev: 0x5603f55b24e0
	*/


	std::cout << "------- SETTING arr2 = nullptr -------" << std::endl;

	arr2 = nullptr;

	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

	/*
				--- SNIPPET OF DEBUG LOGS ---
	Chunks:
		Chunk at: 0x5603f55b2400, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2450, Prev: 0
		Chunk at: 0x5603f55b2450, Size: 24 bytes, Free, gc_mark : UNMARKED, Next: 0x5603f55b2490, Prev: 0x5603f55b2400
		Chunk at: 0x5603f55b2490, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b24e0, Prev: 0x5603f55b2450
		Chunk at: 0x5603f55b24e0, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2530, Prev: 0x5603f55b2490
		Chunk at: 0x5603f55b2530, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0, Prev: 0x5603f55b24e0

	*/


	std::cout << "------- SETTING a,b,c = nullptr -------" << std::endl;

	a = nullptr;
	b = nullptr;
	c = nullptr;

	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

	/*

			--- SNIPPET OF DEBUG LOGS ---

	[INFO]    Starting sweeping phase..

	[INFO]          Sweeping pointer -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b2490

	[INFO]          Coalescing with previous chunk -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b24e0

	[INFO]          Coalescing with previous chunk -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b2530

	[INFO]          Coalescing with previous chunk -> 0x5603f55b2450

	[INFO]          Sweeping pointer -> 0x5603f55b2450

	[INFO]    Finished Sweeping phase

	Chunks:
		Chunk at: 0x5603f55b2400, Size: 40 bytes, Allocated, gc_mark : MARKED, Next: 0x5603f55b2450, Prev: 0
		Chunk at: 0x5603f55b2450, Size: 264 bytes, Free, gc_mark : UNMARKED, Next: 0, Prev: 0x5603f55b2400
	*/

}