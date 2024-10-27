#ifndef BST_NODE_H
#define BST_NODE_H

#include <cstddef>

class BST_Node {
public:

	void* chunk_ptr;
	std::size_t chunk_size;
	BST_Node* left;
	BST_Node* right;

	BST_Node(void* chunk_ptr, std::size_t chunk_size);
};

#endif