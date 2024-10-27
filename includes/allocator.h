#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef>
#include <unistd.h>
#include "chunk_metadata.h"
#include "bst_node.h"
#include <sstream>
#include <string>

class Allocator{
private:
	static const std::size_t INITIAL_HEAP_CAPACITY = 1024 * 1024; 	// 1 MB initial heap
	void* heap_start;
	std::size_t used_heap_size;
	std::size_t HEAP_CAPACITY;

	static const std::size_t MAX_NODES = 1024;
	BST_Node* node_pool;
	bool* node_used;
	std::size_t node_index;

	BST_Node* allocate_node(std::size_t size, void* chunk);
	void deallocate_node(BST_Node* node);

	BST_Node* allocated_chunks_root = nullptr;

	BST_Node* insert_in_bst(BST_Node* root, void* chunk_ptr, std::size_t chunk_size);
	BST_Node* search_ptr_in_bst(BST_Node* root, void* chunk_ptr);
	BST_Node* remove_node_in_bst(BST_Node* root, void* chunk_ptr);
	BST_Node* find_min_node(BST_Node* node);
	void print_bst(BST_Node* root, int space = 0, int height = 10);

	int expand_heap(std::size_t size);

	
	void log_info(const std::string& str);

	// Private constructor to prevent direct instantiation
	Allocator(bool debug_mode);

	// Disable copy constructor and assignment operator
	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;
	std::ostringstream out;
	bool DEBUG_MODE = false;

public:
	static Allocator& getInstance(bool bebug_mode = false);
	void* allocate(std::size_t size);
	void deallocate(void* ptr);
	void heap_dump();
	void print_allocated_chunks();
};

#endif 