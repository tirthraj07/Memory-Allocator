#include <iostream>
#include "allocator.h"

class MyClass {
public:
	MyClass(std::string param) {
		std::cout << "My Class Constructor called" << std::endl;
		std::cout << "Param passed : " << param << std::endl;
	}

	~MyClass() {
		std::cout << "My Class Destructor called" << std::endl;
	}

	void foo() {
		std::cout << "My Class function called" << std::endl;
	}
};


int main(){
	/*
	// Turn on DEBUG MODE for internal heap logs
	// Default DEBUG_MODE = false
	// Note: heap_dump() and print_allocated_chunks() only works in debug mode.
	// To enable debug mode, use the following syntax:

	bool DEBUG_MODE = true;
	Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
	*/

	Allocator& alloc = Allocator::getInstance();


	// only works in debug_mode
	alloc.heap_dump();
	alloc.print_allocated_chunks();
	

	int* int_arr = (int*)alloc.allocate(sizeof(int) * 10);
	for (int i = 0; i < 10; i++) {
		int_arr[i] = i;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << int_arr[i] << " ";
	}
	std::cout << std::endl;

	
	char* char_arr = (char*)alloc.allocate(sizeof(char) * 26);
	
	for (int i = 0; i < 26; i++) {
		char_arr[i] = (char)(65 + i);
	}

	for (int i = 0; i < 26; i++) {
		std::cout << char_arr[i] << " ";
	}
	std::cout << std::endl;

	// expected to have 2 chunks -> int_arr and char_arr
	// int_arr chunk = 10*sizeof(int) + 32 (sizeof(Chunk_Metadata)) = 10*4 + 32 = 40 + 32 = 72 bytes
	// char_arr chunk = 26*sizeof(char) + 32 (sizeof(Chunk_Metadata)) = 26 + 32 = 58 bytes

	alloc.heap_dump();
	alloc.print_allocated_chunks();

	
	// let us free int_arr. It should free -> 10 * sizeof(int) = 10*4 + 32 = 40+32 = 72 bytes
	alloc.deallocate(int_arr);

	// now if we look at heap dump, we will see that chunk 1 is free and allocated_chunk tree should only have one chunk inside of it
	alloc.heap_dump();
	alloc.print_allocated_chunks();

	
	// now let us create a new chunk of char of size 3
	// it will take 3*sizeof(char) + 32 (sizeof(Chunk_Metadata)) =  35 bytes
	// it will be placed in 72 bytes chunk which we deallocted earlier
	// remaining 72-35  = 37 bytes can be used to create a new chunk of size 5 bytes 
	// as 5 bytes + 32 bytes (sizeof(Chunk_Metadata)) = 37 bytes


	char* chunk_1 = (char*) alloc.allocate(sizeof(char) * 3);
	for (int i = 0; i < 3; i++) {
		chunk_1[i] = (char)(65 + i);
	}

	// expected to have 3 chunks : chunk_1 (3 bytes) + free_chunk (5 bytes) + char_arr chunk (26 bytes)
	alloc.heap_dump();
	alloc.print_allocated_chunks();
	
	char* chunk_2 = (char*) alloc.allocate(sizeof(char) * 5);
	for (int i = 0; i < 5; i++) {
		chunk_2[i] = (char)(65 + i);
	}

	for (int i = 0; i < 5; i++) {
		std::cout << chunk_2[i] << " ";
	}
	std::cout << std::endl;

	// expected to have 3 chunks : chunk_1 (3 bytes) + chunk_2 (5 bytes) + char_arr chunk (26 bytes)

	alloc.heap_dump();
	alloc.print_allocated_chunks();
	
	// now let us deallocate all chunks
	// expected that all chunks coalesce
	// coalesced chunk = 3 (chunk_1) + 5 (chunk_2) + 32 (chunk_2 Metadata) + 26 (char_arr) + 32 (char_arr Metadata) = 98 byes

	alloc.deallocate(chunk_1);
	alloc.deallocate(chunk_2);
	alloc.deallocate(char_arr);

	alloc.heap_dump();
	alloc.print_allocated_chunks();
	

	// now let us create the object of class using the allocate_new function which behaves like new operator in C++
	MyClass* myclass_ptr = alloc.allocate_new<MyClass>("Hello");

	alloc.heap_dump();
	alloc.print_allocated_chunks();
	
	myclass_ptr->foo();
	alloc.free_ptr<MyClass>(myclass_ptr);


	alloc.heap_dump();
	alloc.print_allocated_chunks();
}