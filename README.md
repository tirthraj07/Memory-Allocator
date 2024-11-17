# Dynamic Heap Memory Allocator with Integrated Garbage Collection
### By Tirthraj Mahajan

This project is a custom memory allocator in C++ designed from scratch to manage memory with fine control and efficiency, now enhanced with an integrated mark-and-sweep garbage collection mechanism. 
It implements chunk-based memory management using a best-fit allocation strategy and a binary search tree (BST) to track free chunks by size. We leverage the sbrk system call to dynamically manage heap space, providing both expansion and shrinkage as needed.

To reduce internal fragmentation, the allocator coalesces adjacent free chunks, combining them into larger blocks. 
This coalescing strategy optimizes memory usage by preventing small, unusable chunks from scattering across the heap, ensuring more contiguous memory blocks are available for future allocations. 
Additionally, the allocator provides a custom allocate_new function that uses templates and placement syntax, allowing for efficient object construction directly within allocated memory.

### **Garbage Collection Process**  
The **mark-and-sweep garbage collection** is implemented using **Depth-First Search (DFS)** to traverse the object graph during the mark phase. The allocator identifies reachable memory chunks starting from a set of GC roots and marks them as "in use." In the sweep phase, a **sliding window algorithm** scans the heap for unmarked memory chunks, reclaiming unused memory and coalescing adjacent free chunks to optimize space usage.

This project employs a **stop-the-world garbage collection** approach, meaning that during garbage collection, the execution of the program is temporarily paused. This ensures the integrity of the memory being managed, as no new allocations or deallocations occur while the mark-and-sweep algorithm is in progress. Although this approach simplifies the implementation and guarantees correctness, it may introduce brief pauses in execution, making it more suitable for systems where occasional interruptions are acceptable.

This integration of garbage collection into the allocator enhances its robustness by automating memory management while maintaining fine-grained control and efficiency. It exemplifies a blend of classic algorithms, modern optimization techniques, and foundational principles of memory management, paving the way for further innovation in custom allocator design.

## Strategies Used

1. **Chunk-based Memory Management**: Memory is divided into chunks, each managed by metadata containing allocation status, size, and neighboring chunk information for merging.
2. **Best-Fit Allocation**: To reduce fragmentation, the allocator searches for the best-fitting free chunk that matches the requested size.
3. **Binary Search Tree (BST)**: A BST is used to organize free chunks efficiently by size for allocation and by pointer for deallocation, optimizing memory access.
4. **Mark-and-Sweep Garbage Collection** : Ensures unused memory is reclaimed automatically, reducing memory leaks and simplifying memory management.

---

## Project Structure

The project’s structure includes header files, implementation files, and a main entry point to compile and run the allocator.

```
root
├── includes
│   ├── allocator.h         # Header for Allocator class, containing main allocation methods
│   ├── chunk_metadata.h    # Header for Chunk_Metadata class, tracking chunk data
│   ├── garbage_collector.h # Header for Garbage_Collector Class, for garbage collection process
│   └── bst_node.h          # Header for BST_Node class, managing BST nodes for chunk management
│
├── lib
│   ├── allocator.cpp       	# Implementation of Allocator class functions
│   ├── chunk_metadata.cpp  	# Implementation of Chunk_Metadata functions
│   ├── garbage_collector.cpp 	# Implementation of Garbage_collection functions
│   └── bst_node.cpp        	# Implementation of BST_Node functions
│
├── src
│   └── main.cpp            # Main entry point, testing memory allocation and deallocation
│
├── CMakeLists.txt          # CMake build configuration
└── Dockerfile              # Docker configuration to run on non-Linux systems
```

---

## Running the Project

### Requirements
This project uses `<unistd.h>`, a library primarily available on Linux systems. 

#### For Linux Users
1. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ./MemoryAllocator
   ```

#### For Other OS Users
Use Docker to run the project:
1. Build and run the Docker container:
   ```bash
   docker build -t cpp_application .
   docker run -it --name allocator -v .:/app cpp_application
   ```

---

## Examples

### Example 1: Basic Memory Allocation
**Title**: Allocating and Deallocating Memory

**Description**: This example demonstrates allocating memory for an integer array and then deallocating it.

**Code**:
```cpp
#include "allocator.h"

int main(){

	Allocator& alloc = Allocator::getInstance();		        // Get the instance of allocator 
	alloc.GC_ENABLED = false;									// Disable automatic Garbage Collection for this example
	
	int* int_arr = (int*)alloc.allocate(sizeof(int) * 10);		// Allocate 10 integers in heap
	

	for (int i = 0; i < 10; i++) {
		int_arr[i] = i;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << int_arr[i] << " ";
	}
	std::cout << std::endl;

	alloc.deallocate(int_arr);

}
```

**Output**:
```
0 1 2 3 4 5 6 7 8 9
```

**Explanation**: The allocator requests space for 10 integers, prints allocation details, and then successfully deallocates the memory.

### Example 2: Basic Memory Allocation With GC Enabled
**Title**: Allocating and Deallocating Memory with automatic garbage collection on

**Description**: This example demonstrates allocating memory for an integer array and then deallocating it.

**Code**:
```cpp

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

```

**Output**:
```
0 1 2 3 4 5 6 7 8 9
```

### Example 3: Create and Destroy Object
**Title**: Implementation of 'new' and 'delete' keyword

**Description**: This example demonstrates how to create and delete custom objects

**Code**:
```cpp
#include <iostream>
#include "allocator.h"
#include "garbage_collector.h"

// Example Class to showcase Allocator and GC
class MyClass {
public:
	int myint;

	MyClass(int myint): myint(myint) {
		std::cout << "My Class Constructor Called" << std::endl;
		std::cout << "My Class ID = " << myint << std::endl;
	}

	~MyClass(){
		std::cout << "My Class Destructor Called" << std::endl;
		std::cout << "My Class ID = " << myint << std::endl;
	}

	void foo() {
		std::cout << "My Class Function Called" << std::endl;
		std::cout << "My Class ID = " << myint << std::endl;
	}

};


int main(){
	// By Default, DEBUG_MODE = false. To enable debug logs, you can do DEBUG_MODE = true
	/*
		bool DEBUG_MODE = true;
		Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
	*/

	Allocator& alloc = Allocator::getInstance();

	// By Default, GC_ENABLED = true. You can automatic garbage collection by setting GC_ENABLED = false;

	alloc.GC_ENABLED = true;

	// If you want to manually invoke GC, you can get the instance of GC from the allocator.

	Garbage_Collector& gc = alloc.getGC();

	// Let us create an array of size 3 with MyClass Objects

	MyClass** arr = (MyClass**)alloc.allocate(3 * sizeof(MyClass*), (void**)&arr);

	for (int i = 0; i < 3; i++) {
		// The first argument of allocate_new is the stack variable reference which provides root for GC
		// If provided, the GC will not collect the area referenced by the stack variable during garbage collection
		// You can decide to not provide it by setting the 1st argument = nullptr. As long as the parent node is referencing the heap, the gc will not collect any of the child nodes
		// The next 'n' arguments are the arguments for the constructor. The arguments are forwarded to the constructor
		// Here there is only 1 var in Constructor, id. So we will provide the id
		int myObjId = i;
		arr[i] = alloc.allocate_new<MyClass>(nullptr, myObjId);
	}
	/*
	OUTPUT:
	My Class Constructor Called
	My Class ID = 0

	My Class Constructor Called
	My Class ID = 1

	My Class Constructor Called
	My Class ID = 2
	*/



	// Let us invoke the foo() function
	for (int i = 0; i < 3; i++) {
		arr[i]->foo();
	}

	/*
	OUTPUT:
	My Class Function Called
	My Class ID = 0

	My Class Function Called
	My Class ID = 1

	My Class Function Called
	My Class ID = 2

	*/

	// Let us look at snippet of heap
	alloc.heap_dump();		// Only works if DEBUG_MODE = true
	/*
	Chunks:
		Chunk at: 0x5587e0d5e400, Size: 24 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5587e0d5e440, Prev: 0
		Chunk at: 0x5587e0d5e440, Size: 4 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5587e0d5e46c, Prev: 0x5587e0d5e400
		Chunk at: 0x5587e0d5e46c, Size: 4 bytes, Allocated, gc_mark : UNMARKED, Next: 0x5587e0d5e498, Prev: 0x5587e0d5e440
		Chunk at: 0x5587e0d5e498, Size: 4 bytes, Allocated, gc_mark : UNMARKED, Next: 0, Prev: 0x5587e0d5e46c
	*/

	// Now if we want to assign the memory to variable, you can do so using the 'assign' function
	// Assign function ensures that the garbage collector will not pick up the objects during the collection process

	MyClass* ptr = alloc.assign(&ptr, arr[2]);

	// Now if we set the arr to nullptr or some other variable, it will not collect the area pointed by obj
	arr = nullptr;

	// Let us manually invoke the GC
	gc.gc_collect();
	alloc.heap_dump();		// Only works if DEBUG_MODE = true
	/*
	Chunks:
		Chunk at: 0x559db2cbd400, Size: 112 bytes, Free, gc_mark : UNMARKED, Next: 0x559db2cbd498, Prev: 0
		Chunk at: 0x559db2cbd498, Size: 4 bytes, Allocated, gc_mark : MARKED, Next: 0, Prev: 0x559db2cbd400
	*/
	// As you can see, all the unreachable chunks have been collected by the gc

	// Important Note: Garbage collector does not invoke the destructor of the object
	// To safely destroy an object. Use the free_ptr() function

	alloc.free_ptr<MyClass>(ptr);
	/*
	My Class Destructor Called
	My Class ID = 2
	*/

	alloc.heap_dump();		// Only works if DEBUG_MODE = true
	/*
	Chunks:
		Chunk at: 0x559db2cbd400, Size: 156 bytes, Free, gc_mark : UNMARKED, Next: 0, Prev: 0
	*/

}
```

**Output**:
```
My Class Constructor Called
My Class ID = 0
My Class Constructor Called
My Class ID = 1
My Class Constructor Called
My Class ID = 2
My Class Function Called
My Class ID = 0
My Class Function Called
My Class ID = 1
My Class Function Called
My Class ID = 2
My Class Destructor Called
My Class ID = 2
```

**Explanation**: Use the `allocate_new()` method to instantiate a new object. Pass the parameters as the arguments to the constructor

### Example 4: Enable DEBUG Mode
**Title**: Turn on DEBUG Mode

**Description**: To see the internal logs and working enable DEBUG Mode

**Code**:
```cpp

int main(){

	// By Default, DEBUG_MODE = false. To enable debug logs, you can do DEBUG_MODE = true
	
	bool DEBUG_MODE = true;
	Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
	
}
```
---

## How It Works Internally

![internal-structure](public/allocator_diagram.png)

### `brk` and `sbrk` System Calls
The allocator uses the `sbrk` system call, which adjusts the program's data space by changing the program break location. By controlling `sbrk`, the allocator directly manages memory allocation outside of the standard C++ heap allocation (e.g., `new` or `malloc`), giving granular control over the memory lifecycle.

### Chunk Allocation Pool and BST Organization
The allocator creates a pool of chunk pointers of allocated chunks managed by a binary search tree (BST). Each chunk has metadata, stored in `Chunk_Metadata`, that tracks the chunk's size, allocation status, and neighboring chunks. 
- **Pointer-based Search**: When deallocating, the BST uses the pointer to locate chunks quickly, allowing efficient deallocation.

### Memory Allocation and Deallocation Process
1. **Allocation**: The allocator searches for an available chunk that best matches the request size using the BST.
   - If no matching chunk is found, `sbrk` is called to expand the heap and create a new chunk.
2. **Deallocation**: The allocator deallocates a chunk and merges it with neighboring free chunks if possible, optimizing memory utilization.

### The `allocate_new` Function
The allocator uses the `allocate_new` function to allocate objects with constructor calls. It combines templates and the `placement new` syntax to directly construct objects in allocated memory without extra allocation overhead. This function exemplifies low-level memory management while providing flexibility to allocate custom object types efficiently.

### The `free_ptr` Function   
The allocator uses the `free_ptr` function to destroy the object pointed by the pointer. It first, calls the destructor and then makes the memory as free

---
