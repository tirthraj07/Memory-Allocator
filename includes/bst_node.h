#ifndef BST_NODE_H
#define BST_NODE_H
#pragma once

#include <cstddef>

/**
 * @class BST_Node
 * @brief Represents a node in the binary search tree used to manage allocated memory chunks.
 *
 * Each node in the BST contains information about a specific allocated memory chunk,
 * including its pointer, size, and pointers to left and right child nodes.
 */
class BST_Node {
public:

	void* chunk_ptr;			///< Pointer to the memory chunk represented by this node.
	std::size_t chunk_size;		///< Size of the memory chunk.
	BST_Node* left;				///< Pointer to the left child node in the BST.
	BST_Node* right;			///< Pointer to the right child node in the BST.


	/**
	 * @brief Constructs a BST_Node with the specified memory chunk pointer and size.
	 * @param chunk_ptr Pointer to the memory chunk.
	 * @param chunk_size Size of the memory chunk.
	 */
	BST_Node(void* chunk_ptr, std::size_t chunk_size);
};

#endif