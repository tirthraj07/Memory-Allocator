#include "bst_node.h"

BST_Node::BST_Node(void* chunk_ptr, std::size_t chunk_size)
{
	this->chunk_ptr = chunk_ptr;
	this->chunk_size = chunk_size;
	this->left = nullptr;
	this->right = nullptr;
}
