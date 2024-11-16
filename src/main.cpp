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

	for (int i = 0; i < 10; i++) {
		arr[i] = i;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << arr[i] << " ";
	}

	std::cout << std::endl;
	
	gc.gc_dump();
	gc.gc_collect();

	int** arr2 = (int**)alloc.allocate(3 * sizeof(int*), (void**)&arr2);
	std::cout << "&arr2 = " << &arr2 << std::endl;
	std::cout << "arr2 = " << arr2 << std::endl;

	for (int i = 0; i < 3; i++) {
		arr2[i] = (int*)alloc.allocate(10 * sizeof(int));
		for (int j = 0; j < 10; j++) {
			arr2[i][j] = (i + 1) * j;
		}
	}
	
	std::cout << "&arr2 = " << &arr2 << std::endl;
	std::cout << "arr2 = " << arr2 << std::endl;

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

	int* a = alloc.assign(&a, arr2[0]);
	int* b = alloc.assign(&b, arr2[1]);
	int* c = alloc.assign(&c, arr2[2]);

	std::cout << "------- SETTING arr2 = nullptr -------" << std::endl;

	arr2 = nullptr;

	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

	std::cout << "------- SETTING a,b,c = nullptr -------" << std::endl;

	a = nullptr;
	b = nullptr;
	c = nullptr;

	gc.gc_dump();
	gc.gc_collect();
	alloc.heap_dump();

}