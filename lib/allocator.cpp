#include "allocator.h"
#include <iostream>
#include <cassert>
#include <cstddef>
#include <string>
#include <unistd.h>
#include "chunk_metadata.h"
#include "bst_node.h"
#include <iomanip>
#include <garbage_collector.h>

#define LBR '\n'

Allocator::Allocator(bool debug_mode):gc(Garbage_Collector::getInstance(debug_mode)), DEBUG_MODE(debug_mode)
{       
    out << "INITILIZATING NODE POOL.." << LBR;
    log_info(out.str());

    this->node_index = 0;
    this->node_pool = static_cast<BST_Node*>(sbrk(MAX_NODES * sizeof(BST_Node)));
    node_used = static_cast<bool*>(sbrk(MAX_NODES * sizeof(bool)));

    if (node_pool == (void*)-1 || node_used == (void*)-1) {
        std::cerr << "Failed to initialize node pool" << LBR;
        exit(1);
    }

    std::fill(node_used, node_used + MAX_NODES, false);
    

    out << "INITILIZATING HEAP.. " <<LBR;
    log_info(out.str());
    out << "INITIAL HEAP CAPACITY " << INITIAL_HEAP_CAPACITY << LBR;
    log_info(out.str());
    out << "Chunk Metadata Size : " << sizeof(Chunk_Metadata) << LBR;
    log_info(out.str());
    heap_start = sbrk(INITIAL_HEAP_CAPACITY);
    if (heap_start == (void*)-1) {
        std::cerr << "Failed to allocate initial heap space" << LBR;
        exit(1);
    }

    HEAP_CAPACITY = INITIAL_HEAP_CAPACITY;
    used_heap_size = 0;

    out<<"Heap initialized at heap_start : " << heap_start << " with capacity of " << HEAP_CAPACITY << LBR;
    log_info(out.str());
}

Allocator& Allocator::getInstance(bool debug_mode)
{    
    static Allocator instance(debug_mode); // Static instance created only once
    return instance;
}

// Private API called by Public allocate() API to prevent users from disabling gc_collect_flag
void* Allocator::allocate(std::size_t size, bool gc_collect_flag)
{
    if (DEBUG_MODE) std::cout << "\n\n\n" << LBR;

    out << "Received Allocation Request for " << size << LBR;
    log_info(out.str());

    if (size <= 0) {
        return nullptr;
    }
    
    if (used_heap_size + size + sizeof(Chunk_Metadata) >= HEAP_CAPACITY) {
        out << "Heap Size not sufficient: used_heap_size + size + sizeof(Chunk_Metadata) >= HEAP_CAPACITY " << used_heap_size + size + sizeof(Chunk_Metadata) << LBR;
        log_info(out.str());

        // If there is no free space, then call the collect method in garbage collector
        if (gc_collect_flag){
            out << "Calling Garbage Collector to collect free space" << LBR;
            log_info(out.str());
            gc.gc_collect();
            return allocate(size, false);
        }

        // If there is still no space after gc collect then expand memory
        if (expand_heap(size + sizeof(Chunk_Metadata)) != 0) {
            // If OS does not provide more memory -> Throw error
            std::cerr << "Error: HEAP OVERFLOW" << LBR;
            exit(1);
        }
    }

    // first chunk entry in heap    
    if (used_heap_size == 0) {
        out << "Creating first chunk " << LBR;
        log_info(out.str());

        Chunk_Metadata* metadata = reinterpret_cast<Chunk_Metadata*>(heap_start);
        metadata->chunk_size = size;
        metadata->next = nullptr;
        metadata->prev = nullptr;
        metadata->is_free = false;

        // Update the used_heap_size to include metadata and the requested chunk
        used_heap_size = sizeof(Chunk_Metadata) + size;

        // Return the pointer to the start of the chunk's data (after metadata)
        void* chunk_ptr = reinterpret_cast<void*>(
            reinterpret_cast<char*>(heap_start) + sizeof(Chunk_Metadata)
        );

        allocated_chunks_root = insert_in_bst(allocated_chunks_root, chunk_ptr, size);

        return chunk_ptr;
    }

    // now if the used_heap_size is not 0
    // the look for free chunks in the heap first 
    // linear search the heap from heap_start to heap_start + used_heap_size 
    // to iterate, retrieve back the heap metadata from heap_start and do while(metadata_ptr->next != NULL)
    // check if the is_free flag is true and if the chunk_size >= size
    // we need to implement 'best-fit' stratergy
    // so iterate though all the chunks and find the least free chunk with chunk_size >= size
    // if the chunk_size == size then retrieve back the chunk pointer from metdatadata's currentChunk() function
    // if the chunk_size > size, check if the chunk_size - size > sizeof(Chunk_Metadata) 
    // if it is greater then create a new chunk immediatly after the chunk and insert the metadata as follows
    // let the is_free = true for that
    // let chunk_size = previous chunk_size - new chunk_size - sizeof(Chunk_Metadata)
    // handle the next and prev pointers of chunk metadata
    // If no chunk was found, append the metadata to the end of chunk and return the currentChunk() ptr

    Chunk_Metadata* best_fit = nullptr;
    Chunk_Metadata* last_chunk = reinterpret_cast<Chunk_Metadata*>(heap_start);
    Chunk_Metadata* current = reinterpret_cast<Chunk_Metadata*>(heap_start);

    // Linear search through the allocated chunks
 
    while (current!=nullptr && reinterpret_cast<char*>(current) < reinterpret_cast<char*>(heap_start) + used_heap_size) {
        if (current->is_free && current->chunk_size >= size) {
            if (!best_fit || current->chunk_size < best_fit->chunk_size) {
                best_fit = current; // Update best fit
            }
        }
        last_chunk = current;
        current = current->next; // Move to the next chunk
    }

    // If a suitable free chunk was found
    if (best_fit) {
        out << "Best fit Found" << LBR
            << " best_fit->chunk_size=" << best_fit->chunk_size << LBR
            << " requested chunk_size=" << size << LBR;
        log_info(out.str());

        // If the chunk size exactly matches the requested size
        if (best_fit->chunk_size == size) {
            out << "Perfect Fit Found" << LBR;
            log_info(out.str());
            best_fit->is_free = false; // Mark as allocated
            void* chunk_ptr = best_fit->currentChunk();
            allocated_chunks_root = insert_in_bst(allocated_chunks_root, chunk_ptr, size);
            return chunk_ptr;
        }

        // If the chunk size is larger than the requested size, split the chunk
        if (best_fit->chunk_size > size) {
            int remaining_size = best_fit->chunk_size - size - sizeof(Chunk_Metadata);

            out << "Imperfect Fit Found" << LBR
                << " remaining_size=" << remaining_size << LBR
                << " sizeof(Chunk_Metadata)=" << sizeof(Chunk_Metadata) << LBR;
            log_info(out.str());

            // Ensure the remaining chunk is large enough to hold metadata
            if (remaining_size > 0) {
                // Create a new chunk immediately after the best fit chunk
                out << "Request for new chunk creation" << LBR;
                log_info(out.str());

                Chunk_Metadata* new_chunk = reinterpret_cast<Chunk_Metadata*>(
                    reinterpret_cast<char*>(best_fit) + sizeof(Chunk_Metadata) + size
                );

                new_chunk->chunk_size = remaining_size;
                new_chunk->is_free = true;

                new_chunk->next = best_fit->next; 
                new_chunk->prev = best_fit; 
                best_fit->next = new_chunk; 

                if (new_chunk->next) {
                    new_chunk->next->prev = new_chunk; 
                }

                best_fit->chunk_size = size;

                out << "New chunk created at " << new_chunk << LBR
                    << " is_free=" << new_chunk->is_free << LBR
                    << " chunk_size=" << new_chunk->chunk_size << LBR
                    << " new_chunk->next=" << new_chunk->next << LBR
                    << " new_chunk->prev=" << new_chunk->prev << LBR;
                log_info(out.str());                
            }
        }

        best_fit->is_free = false; 
        void* chunk_ptr = best_fit->currentChunk();
        allocated_chunks_root = insert_in_bst(allocated_chunks_root, chunk_ptr, size);

        out << "Best Fit chunk at " << best_fit << LBR
            << " best_fit->is_free=" << best_fit->is_free << LBR
            << " best_fit->chunk_size=" << best_fit->chunk_size << LBR
            << " best_fit->next=" << best_fit->next << LBR
            << " best_fit->prev=" << best_fit->prev << LBR;
        log_info(out.str());

        return chunk_ptr; 
    }

    // If no suitable free chunk was found, append to the end
    Chunk_Metadata* new_chunk = reinterpret_cast<Chunk_Metadata*>(
        reinterpret_cast<char*>(heap_start) + used_heap_size
    );

    new_chunk->chunk_size = size;
    new_chunk->is_free = false; 
    new_chunk->next = nullptr;
    new_chunk->prev = last_chunk;
    last_chunk->next = new_chunk;
   
    used_heap_size += sizeof(Chunk_Metadata) + size;
    void* chunk_ptr = new_chunk->currentChunk();
    allocated_chunks_root = insert_in_bst(allocated_chunks_root, chunk_ptr, size);

    return chunk_ptr;
}

void* Allocator::allocate(std::size_t size)
{
    return allocate(size, true);
}

void Allocator::deallocate(void* ptr)
{
    out << "Received request for deallocation of pointer " << ptr << LBR;
    log_info(out.str());

    // to implement deallocate function
    // first we need to check if the ptr provided is valid or not
    // there are two cases for invalid pointers
    // 1. if the pointer is pointing to the memory location inside of heap but it is not starting, i.e currentChunk()
    // 2. if the pointer is pointing to memory location completely outside the heap
    // we are going to treat both of them the same by throwing an error
    // NOTE: nullptr is valid and thus we also need to check if the ptr is nullptr
    // in case of nullptr, we simply want to end the function as it has no effect
    // we need to iterate through the heap using (current!=nullptr) while loop
    // if the current->currentChunk() == ptr then it is valid
    // after entire iteration of heap, if we do not find the chunk ptr, then exit with error state
    // if we found the chunk to be deallocated, then we need to do the following
    // 1. set the is_free flag = true
    // 2. check if the next chunk is free. If it is, then let the size of the current chunk_size += next chunk size and current next = next chunk next()
    // 3. check if the previous chunk if free. If it is then let the size of the previous chunk += current chunk size and prev chunk next() = current chunk
    // essentially we need to coalesces the free adjacent chunks

    // Check if the pointer is nullptr
    if (ptr == nullptr) {
        return;
    }

    // Check if the pointer is within the heap range
    if (reinterpret_cast<char*>(ptr) < reinterpret_cast<char*>(heap_start) ||
        reinterpret_cast<char*>(ptr) >= reinterpret_cast<char*>(heap_start) + used_heap_size) {
        std::cerr << "Error: Invalid pointer provided to deallocate" << LBR;
        exit(1);
    }

    out << "Verification Done:  " << ptr << " is valid" << LBR;
    log_info(out.str());

    Chunk_Metadata* current = reinterpret_cast<Chunk_Metadata*>(heap_start);
    bool found = false;

    /*while (current != nullptr) {
        if (current->currentChunk() == ptr) {
            found = true;
            break;
        }
        current = current->next;
    }*/

    BST_Node* bst_node = search_ptr_in_bst(allocated_chunks_root, ptr);
    if (bst_node != nullptr) {
        found = true;
        current = reinterpret_cast<Chunk_Metadata*>(reinterpret_cast<char*>(bst_node->chunk_ptr) - sizeof(Chunk_Metadata));
        out << "Pointer found in allocation tree" << LBR
            << " ptr=" << ptr << LBR
            << " chunk_ptr=" << bst_node->chunk_ptr << LBR
            << " bst_chunk_node=" << current << LBR;
        log_info(out.str());
    }

    if (!found) {
        std::cerr << "Error: Pointer does not point to a valid allocated chunk" << LBR;
        exit(1);
    }

    current->is_free = true;
    allocated_chunks_root = remove_node_in_bst(allocated_chunks_root, ptr);

    // Coalescing adjacent free chunks
    if (current->next != nullptr && current->next->is_free) {
        current->chunk_size += current->next->chunk_size + sizeof(Chunk_Metadata);
        current->next = current->next->next; 

        if (current->next != nullptr) {
            current->next->prev = current; 
        }
    }

    if (current->prev != nullptr && current->prev->is_free) {
        current->prev->chunk_size += current->chunk_size + sizeof(Chunk_Metadata);
        current->prev->next = current->next;

        if (current->next != nullptr) {
            current->next->prev = current->prev; 
        }
    }

}

void Allocator::heap_dump()
{
    if(DEBUG_MODE){
        Chunk_Metadata* current = reinterpret_cast<Chunk_Metadata*>(heap_start);
        std::size_t total_allocated = 0;
        std::size_t total_free = 0;
        std::size_t allocated_chunks = 0;
        std::size_t free_chunks = 0;

        std::cout << "----------------------------------------\n"
            << "Heap Dump:\n"
            << "Total Heap Capacity: " << HEAP_CAPACITY << " bytes\n"
            << "Used Heap Size: " << used_heap_size << " bytes\n"
            << "Chunks:\n";

        while (current != nullptr) {
            std::cout << "Chunk at: " << current
                << ", Size: " << current->chunk_size
                << " bytes, "
                << (current->is_free ? "Free" : "Allocated")
                << ", Next: " << current->next
                << ", Prev: " << current->prev
                << "\n";
            
            if (current->is_free) {
                total_free += current->chunk_size;
                free_chunks++;
            }
            else {
                total_allocated += current->chunk_size;
                allocated_chunks++;
            }

            current = current->next; // Move to the next chunk
        }

        std::cout << "Summary:\n"
            << "Total Allocated Memory: " << total_allocated << " bytes\n"
            << "Total Free Memory: " << total_free << " bytes\n"
            << "Number of Allocated Chunks: " << allocated_chunks << "\n"
            << "Number of Free Chunks: " << free_chunks << "\n"
            << "----------------------------------------" << LBR;
    }
}

void Allocator::print_allocated_chunks()
{
    if(DEBUG_MODE){
        std::cout << "-- PRINTING ALLOCATED CHUNKS --" << LBR;
        print_bst(allocated_chunks_root);
    }
}

void Allocator::print_bst(BST_Node* root, int space, int height)
{
    // Base case
    if (root == nullptr) {
        return;
    }

    // Increase distance between levels
    space += height;

    // Process right child first
    print_bst(root->right, space);

    // Print current node after space
    std::cout << LBR;
    std::cout << std::setw(space) << reinterpret_cast<void*>(reinterpret_cast<char*>(root->chunk_ptr) - sizeof(Chunk_Metadata)) << " : " << root->chunk_size; // You can change this to root->size or any other property
    std::cout << LBR;

    // Process left child
    print_bst(root->left, space);
}

BST_Node* Allocator::allocate_node(std::size_t size, void* chunk)
{
    out << "Received request for node allocation: size=" << size << " chunk=" << chunk << LBR;
    log_info(out.str());
    for (std::size_t i = 0; i < MAX_NODES; ++i) {
        out << "Node: i=" << i << LBR;
        log_info(out.str());

        out << " node_pool=" << &node_pool[i] << LBR;
        log_info(out.str());

        out << " node_used=" << node_used[i] << LBR;
        log_info(out.str());

        if (!node_used[i]) {
            out << "Found free node" << LBR;
            log_info(out.str());

            node_used[i] = true;
            node_pool[i] = BST_Node(chunk, size);
            return &node_pool[i];
        }
    }
    return nullptr;
}

void Allocator::deallocate_node(BST_Node* node)
{
    if (node) {
        std::size_t index = node - node_pool; // Calculate the node's index
        node_used[index] = false;             // Mark node as free
    }
}

BST_Node* Allocator::insert_in_bst(BST_Node* root, void* chunk_ptr, std::size_t chunk_size)
{
    out << "Received Request for inserting node in BST: root=" << root << " chunk_ptr=" << chunk_ptr << " chunk_size=" << chunk_size << LBR;
    log_info(out.str());

    if (root == nullptr) {
        return allocate_node(chunk_size, chunk_ptr);
    }
    if (chunk_ptr < root->chunk_ptr) {
        root->left = insert_in_bst(root->left, chunk_ptr, chunk_size);
    }
    else {
        root->right = insert_in_bst(root->right, chunk_ptr, chunk_size);
    }
    return root;
}

BST_Node* Allocator::search_ptr_in_bst(BST_Node* root, void* chunk_ptr)
{
    if (root == nullptr || root->chunk_ptr == chunk_ptr) {
        return root; 
    }

    // Compare the given pointer with the current node's address
    if (chunk_ptr < root->chunk_ptr) {
        // Search in the left subtree
        return search_ptr_in_bst(root->left, chunk_ptr);
    }
    else {
        // Search in the right subtree
        return search_ptr_in_bst(root->right, chunk_ptr);
    }
}

BST_Node* Allocator::remove_node_in_bst(BST_Node* root, void* chunk_ptr)
{
    // Base case: if the tree is empty
    if (root == nullptr) {
        return root; // Return null if the node is not found
    }

    if (chunk_ptr < root->chunk_ptr) {
        root->left = remove_node_in_bst(root->left, chunk_ptr);
    }
    else if (chunk_ptr > root->chunk_ptr) {
        root->right = remove_node_in_bst(root->right, chunk_ptr);
    }
    else {
        // Node with only one child or no child
        if (root->left == nullptr) {
            // Node has no left child
            BST_Node* temp = root->right; 
            deallocate_node(root);
            return temp;
        }
        else if (root->right == nullptr) {
            // Node has no right child
            BST_Node* temp = root->left;
            deallocate_node(root);
            return temp;
        }

        // Node with two children: Get the inorder successor
        BST_Node* temp = find_min_node(root->right);

        root->chunk_ptr = temp->chunk_ptr;
        root->chunk_size = temp->chunk_size;

        // Delete the inorder successor
        root->right = remove_node_in_bst(root->right, temp->chunk_ptr);
    }

    return root; 
}

BST_Node* Allocator::find_min_node(BST_Node* node)
{
    BST_Node* current = node;
    while (current && current->left != nullptr) {
        current = current->left;
    }
    return current;
}

int Allocator::expand_heap(std::size_t size)
{
    if (size <= 0) {
        return 0;  
    }

    std::size_t expansion_size = size * 2;

    void* result = sbrk(expansion_size);
    if (result == (void*)-1) {
        std::cerr << "Error: Failed to expand heap by " << expansion_size << " bytes" << LBR;
        return 1; 
    }

    HEAP_CAPACITY += expansion_size;
    std::cout << "Heap successfully expanded by " << expansion_size
        << " bytes. New HEAP_CAPACITY: " << HEAP_CAPACITY << LBR;

    return 0; 
}

void Allocator::log_info(const std::string& str)
{
    if(DEBUG_MODE){
        std::cout << "[INFO]    " << str << LBR;
    }
    out.str(""); // Clear out the contents after logging
    out.clear();
}