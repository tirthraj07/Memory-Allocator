#include "allocator.h"
#include <iostream>
#include <cassert>
#include <cstddef>
#include <unistd.h>
#include "chunk_metadata.h"

Allocator::Allocator()
{
    std::cout << "INITILIZATING HEAP.. " << std::endl;
    std::cout << "INITIAL HEAP CAPACITY " << INITIAL_HEAP_CAPACITY << std::endl;
    std::cout << "Chunk Metadata Size : " << sizeof(Chunk_Metadata) << std::endl;
    heap_start = sbrk(INITIAL_HEAP_CAPACITY);
    if (heap_start == (void*)-1) {
        std::cerr << "Failed to allocate initial heap space" << std::endl;
        exit(1);
    }

    HEAP_CAPACITY = INITIAL_HEAP_CAPACITY;
    used_heap_size = 0;

    std::cout << "Heap initialized at heap_start : " << heap_start << " with capacity of " << HEAP_CAPACITY << std::endl;

}

void* Allocator::allocate(std::size_t size)
{
    if (size <= 0) {
        return nullptr;
    }
    
    if (used_heap_size + size + sizeof(Chunk_Metadata) >= HEAP_CAPACITY) {
        if (expand_heap(size + sizeof(Chunk_Metadata)) != 0) {
            std::cerr << "Error: HEAP OVERFLOW" << std::endl;
            exit(1);
        }
    }

    // first chunk entry in heap
    if (used_heap_size == 0) {
        Chunk_Metadata* metadata = reinterpret_cast<Chunk_Metadata*>(heap_start);
        metadata->chunk_size = size;
        metadata->next = nullptr;
        metadata->prev = nullptr;
        metadata->is_free = false;

        // Update the used_heap_size to include metadata and the requested chunk
        used_heap_size = sizeof(Chunk_Metadata) + size;

        // Return the pointer to the start of the chunk's data (after metadata)
        return reinterpret_cast<void*>(
            reinterpret_cast<char*>(heap_start) + sizeof(Chunk_Metadata)
            );
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
        // If the chunk size exactly matches the requested size
        if (best_fit->chunk_size == size) {
            best_fit->is_free = false; // Mark as allocated
            return best_fit->currentChunk();
        }

        // If the chunk size is larger than the requested size, split the chunk
        if (best_fit->chunk_size > size) {
            int remaining_size = best_fit->chunk_size - size - sizeof(Chunk_Metadata);

            // Ensure the remaining chunk is large enough to hold metadata
            if (remaining_size > (int)(sizeof(Chunk_Metadata))) {
                // Create a new chunk immediately after the best fit chunk
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
            }
        }

        best_fit->is_free = false; // Mark as allocated
        return best_fit->currentChunk(); // Return the pointer to the data area
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

    return new_chunk->currentChunk();
}

void Allocator::deallocate(void* ptr)
{
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
        std::cerr << "Error: Invalid pointer provided to deallocate" << std::endl;
        exit(1);
    }

    Chunk_Metadata* current = reinterpret_cast<Chunk_Metadata*>(heap_start);
    bool found = false;

    while (current != nullptr) {
        if (current->currentChunk() == ptr) {
            found = true;
            break;
        }
        current = current->next;
    }

    if (!found) {
        std::cerr << "Error: Pointer does not point to a valid allocated chunk" << std::endl;
        exit(1);
    }

    current->is_free = true;

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
    std::cout << "Heap Dump:\n";
    std::cout << "Total Heap Capacity: " << HEAP_CAPACITY << " bytes\n";
    std::cout << "Used Heap Size: " << used_heap_size << " bytes\n";

    Chunk_Metadata* current = reinterpret_cast<Chunk_Metadata*>(heap_start);
    std::size_t total_allocated = 0;
    std::size_t total_free = 0;
    std::size_t allocated_chunks = 0;
    std::size_t free_chunks = 0;

    std::cout << "Chunks:\n";
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

    std::cout << "Summary:\n";
    std::cout << "Total Allocated Memory: " << total_allocated << " bytes\n";
    std::cout << "Total Free Memory: " << total_free << " bytes\n";
    std::cout << "Number of Allocated Chunks: " << allocated_chunks << "\n";
    std::cout << "Number of Free Chunks: " << free_chunks << "\n";
}

int Allocator::expand_heap(std::size_t size)
{
    if (size <= 0) {
        return 0;  
    }

    std::size_t expansion_size = size * 2;

    void* result = sbrk(expansion_size);
    if (result == (void*)-1) {
        std::cerr << "Error: Failed to expand heap by " << expansion_size << " bytes" << std::endl;
        return 1; 
    }

    HEAP_CAPACITY += expansion_size;
    std::cout << "Heap successfully expanded by " << expansion_size
        << " bytes. New HEAP_CAPACITY: " << HEAP_CAPACITY << std::endl;

    return 0; 
}