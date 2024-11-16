#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#pragma once

#include <cstddef>
#include <unistd.h>
#include "chunk_metadata.h"
#include "bst_node.h"
#include <sstream>
#include <string>
#include <iostream>
#include <garbage_collector.h>
/**
 * @class Allocator
 * @brief A custom memory allocator implementing a best-fit memory allocation strategy with a singleton pattern.
 *
 * The Allocator manages a heap using the `sbrk` system call for dynamic memory allocation,
 * while also maintaining a binary search tree (BST) to efficiently manage and search allocated memory chunks.
 * The allocator is implemented as a singleton, ensuring only one instance can exist throughout the application.
 */
class Allocator{

// PUBLIC APIs
public:
	/**
	 * @brief Returns the singleton instance of the Allocator.
	 * @param debug_mode Enables or disables debug mode. Default is false.
	 * @return Reference to the singleton instance.
	 */
	static Allocator& getInstance(bool bebug_mode = false);
	
	/**
	 * @brief Allocates memory of a specified size from the heap.
	 * @param size The size of memory to allocate.
	 * @return Pointer to the allocated memory, or nullptr if allocation fails.
	 */

	void* allocate(std::size_t size, void** root=NULL);

	/**
	 * @brief Deallocates memory pointed to by a specified pointer.
	 * @param ptr Pointer to the memory to deallocate.
	 */
	void deallocate(void* ptr);

	/**
 	 * @brief Dumps the current state of the heap, including allocated chunks and free space.
	 *
	 * This function provides a detailed view of the heap's structure, showing
	 * which memory regions are allocated or free. It is only available
	 * when DEBUG_MODE is set to true.
	 */
	void heap_dump();
	
	/**
	 * @brief Prints the list of allocated memory chunks in the heap.
	 *
	 * This function outputs information about each allocated chunk, including
	 * its size and location in memory. It is only available
	 * when DEBUG_MODE is set to true.
	 */
	void print_allocated_chunks();

	/*
		Note the following function body have to be here only
		template functions need to be defined in the header file (or at least included in the same translation unit as their usage).
		Since templates are instantiated at compile-time, the compiler needs the complete definition when generating code for each instantiation, which it doesn’t have if the definition is in a separate .cpp file.
	*/

	/**
	 * @brief Allocates memory for an object of type T and constructs it using
	 *        the provided arguments.
	 *
	 * This function uses a custom memory allocator to allocate sufficient
	 * memory for an object of type T. It then constructs the object in
	 * the allocated memory using the provided arguments. This is done
	 * using placement new, allowing the constructor of T to be called
	 * directly in the allocated memory region.
	 *
	 * @tparam T The type of the object to be allocated.
	 * @tparam Args The types of the arguments to be forwarded to the
	 *               constructor of T.
	 * @param args The arguments to be forwarded to the constructor of T.
	 * @return T* A pointer to the constructed object of type T, or
	 *             nullptr if the allocation fails.
	 */
	template <typename T, typename... Args>
	T* allocate_new(Args&&... args) {
		void* memory = allocate(sizeof(T));
		if (!memory) {
			std::cerr << "Bad allocation Error" << std::endl;
			return nullptr;
		}

		T* obj_ptr = static_cast<T*>(memory);

		// Manually invoke the constructor using placement syntax
		new (obj_ptr) T(std::forward<Args>(args)...);
		return obj_ptr;
	}

	/**
	 * @brief Deallocates memory for an object of type T and calls its destructor.
	 *
	 * This function first checks if the provided pointer is valid (non-null).
	 * If valid, it manually invokes the destructor for the object, ensuring
	 * that any necessary cleanup is performed. After the destructor has
	 * been called, the memory allocated for the object is released back
	 * to the allocator.
	 *
	 * @tparam T The type of the object being deallocated.
	 * @param ptr A pointer to the object of type T that is to be freed.
	 */
	template <typename T>
	void free_ptr(T* ptr) {
		if (ptr) {
			// Manually call the destructor
			ptr->~T();

			// Release memory back to Allocator
			Allocator& alloc = Allocator::getInstance();
			alloc.deallocate(ptr);
		}
	}

	Garbage_Collector& getGC();
	
	template <typename T>
	T* assign(T** dest, T* src) {
		out << "Called assign for dest = " << dest << " , src = " << src << '\n';
		log_info();
		// Update the destination pointer
		*dest = src;

		// Track the destination variable in the GC roots list
		gc->add_gc_roots(reinterpret_cast<void**>(dest));

		// Return the updated destination
		return *dest;
	}

	friend class Garbage_Collector;
	friend class Chunk_Metadata;
	
private:
	static const std::size_t INITIAL_HEAP_CAPACITY = 1024 * 1024; 	///< Initial heap capacity (1 MB).
	void* heap_start;												///< Starting address of the heap.
	std::size_t used_heap_size;										///< The total amount of memory used in the heap.
	std::size_t HEAP_CAPACITY;										///< The current capacity of the heap.

	Garbage_Collector* gc;

	static const std::size_t MAX_NODES = 1024;						///< Maximum number of chunk pointers in the node pool.
	BST_Node* node_pool;											///< Pool of BST nodes used to manage allocated chunks.
	bool* node_used;												///< Boolean array indicating the usage of nodes in node_pool.
	std::size_t node_index;											///< Tracks the next available node in the node pool.
		

	BST_Node* allocated_chunks_root = nullptr;						///< Root of the BST for allocated chunks.

	/**
	 * @brief Private constructor to enforce the singleton pattern.
	 * @param debug_mode Enables or disables debug logging.
	 */
	Allocator(bool debug_mode);

	// Disable copy constructor and assignment operator to enforce singleton pattern
	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;

	void* allocate(std::size_t size, bool gc_collect_flag);

	/**
	 * @brief Allocates a BST node for a memory chunk.
	 * @param size The size of the chunk.
	 * @param chunk Pointer to the chunk memory.
	 * @return Pointer to the allocated BST node.
	 */
	BST_Node* allocate_node(std::size_t size, void* chunk);

	/**
	 * @brief Deallocates a BST node, marking it as available for reuse.
	 * @param node The BST node to be deallocated.
	 */
	void deallocate_node(BST_Node* node);

	/**
	 * @brief Inserts a new chunk into the BST.
	 * @param root The root node of the BST.
	 * @param chunk_ptr Pointer to the chunk memory.
	 * @param chunk_size Size of the chunk.
	 * @return Pointer to the root node of the BST.
	 */
	BST_Node* insert_in_bst(BST_Node* root, void* chunk_ptr, std::size_t chunk_size);

	/**
	 * @brief Searches for a memory chunk in the BST by its pointer.
	 * @param root The root node of the BST.
	 * @param chunk_ptr Pointer to the chunk memory to search.
	 * @return Pointer to the node containing the chunk if found, nullptr otherwise.
	 */
	BST_Node* search_ptr_in_bst(BST_Node* root, void* chunk_ptr);

	/**
	 * @brief Removes a node from the BST.
	 * @param root The root node of the BST.
	 * @param chunk_ptr Pointer to the chunk memory to remove.
	 * @return The new root of the BST after removal.
	 */
	BST_Node* remove_node_in_bst(BST_Node* root, void* chunk_ptr);

	/**
	 * @brief Finds the minimum node in the BST.
	 * @param node The node to start the search.
	 * @return Pointer to the minimum node.
	 */
	BST_Node* find_min_node(BST_Node* node);

	/**
	 * @brief Prints the structure of the BST.
	 * @param root The root of the BST.
	 * @param space Indentation level for visualization.
	 * @param height The maximum height to display.
	 */
	void print_bst(BST_Node* root, int space = 0, int height = 10);

	/**
	* @brief Expands the heap by a specified size.
	* @param size The size to expand the heap by.
	* @return The new heap capacity after expansion.
	*/
	int expand_heap(std::size_t size);

	std::ostringstream out;											///< Output stream for logging purposes.
	bool DEBUG_MODE = false;										///< Indicates if debugging mode is enabled.

	/**
	 * @brief Logs informational messages if debugging is enabled.
	 * @param str The message to log.
	 */
	void log_info();

	Chunk_Metadata* get_chunk(void* ptr);
	void gc_unmark_chunks();
	void find_chunks_within_chunk(Chunk_Metadata* top, void* root_chunk_list[], int& root_chunk_list_size);
	void gc_sweep();
};

#endif 