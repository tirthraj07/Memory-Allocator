#include "garbage_collector.h"
#include "allocator.h"
#include "chunk_metadata.h"

#include <sstream>
#include <string>
#include <iostream>


#define LBR '\n'
#define MAX_ARRAY_CAP 1000



Garbage_Collector::Garbage_Collector(bool debug_mode, void* heap_start, size_t HEAP_CAPACITY):heap_start(heap_start), HEAP_CAPACITY(HEAP_CAPACITY), DEBUG_MODE(debug_mode) {
    out << "Garbage Collector Instantiated" << LBR;
    log_info();
    out << "HEAP_START : " << heap_start << LBR;
    log_info();
    out << "HEAP_CAPACITY : " << HEAP_CAPACITY << LBR;
    log_info();

}

void Garbage_Collector::log_info(){
    if (DEBUG_MODE) {
        std::string str = out.str();
        std::cout << "[INFO]    " << str << LBR;
    }
    out.str(""); // Clear out the contents after logging
    out.clear();
}

bool Garbage_Collector::is_pointer_within_heap(void* ptr)
{
    out << "Called is_pointer_within_heap for ptr = " << ptr << LBR;
    log_info();
    

    char* address = reinterpret_cast<char*>(ptr);
    char* heap_start_addr = reinterpret_cast<char*>(heap_start);
    char* heap_end_addr = heap_start_addr + HEAP_CAPACITY;

    out << "HEAP_START = " << heap_start << LBR;
    log_info();
    out << "HEAP_END = " << (void*)heap_end_addr << LBR;
    log_info();
    out << "Address = " << (void*)address << LBR;
    log_info();


    // Check if the pointer is within the heap bounds
    return address >= heap_start_addr && address < heap_end_addr;
}

void Garbage_Collector::get_roots() {
    out << "get_roots() called" << LBR;
    log_info();

    // Reset the root list size for this round
    root_chunk_list_size = 0;

    // Variable `j` is used to compact the list of potential stack variables containing roots for next round
    int j = 0;

    // Iterate through all potential stack variables that may contain pointers to heap chunks
    for (int i = 0; i < potential_roots_size; i++) {
        // Fetch the potential root pointer from the list
        void** potential_root = potential_stack_vars_containing_roots_list[i];

        // Skip this iteration if the potential root is a null pointer
        if (potential_root == nullptr) {
            continue;
        }

        out << i << ". Potential root = " << (void*)potential_root
            << ", *Potential root = " << *potential_root << LBR;
        log_info();

        // Check if the dereferenced value (the actual pointer) lies within the heap's boundaries
        if (is_pointer_within_heap(*potential_root)) {
            // If it is within the heap, try to retrieve the metadata of the chunk this pointer points to
            Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
            Chunk_Metadata* chunk_ptr = alloc.get_chunk(*potential_root);

            // If a valid chunk is found, add it to the root chunk list
            if (chunk_ptr != nullptr) {
                // Add the pointer to the chunk metadata to the root chunk list
                root_chunk_list[root_chunk_list_size] = reinterpret_cast<void*>(chunk_ptr);
                root_chunk_list_size++;

                // Optimize: Retain only valid roots in the potential stack variables list for the next call
                potential_stack_vars_containing_roots_list[j] = potential_root;
                j++; 
            }
        }
    }

    // Update the size of the potential stack variables list to include only valid roots
    potential_roots_size = j;

    out << "Root list updated. Total roots: " << root_chunk_list_size << LBR;
    log_info();
}

void Garbage_Collector::unmark_chunks()
{
    out << "Called unmarked_chunks().." << LBR
        << "Unmarking chunks..." << LBR;
    log_info();

    Allocator& alloc = Allocator::getInstance();
    alloc.gc_unmark_chunks();
}

void Garbage_Collector::find_chunks_within_chunk(Chunk_Metadata* top)
{
    out << "Searching chunks within >> " << top << LBR;
    log_info();
    Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
    alloc.find_chunks_within_chunk(top, root_chunk_list, root_chunk_list_size);
}

void Garbage_Collector::sweep_phase()
{
    out << "Starting sweeping phase.. " << LBR;
    log_info();
    Allocator& alloc = Allocator::getInstance();
    alloc.gc_sweep();
    out << "Finished Sweeping phase" << LBR;
    log_info();
}



Garbage_Collector& Garbage_Collector::getInstance(void* heap_start, size_t HEAP_CAPACITY, bool debug_mode)
{
    static Garbage_Collector gc(debug_mode, heap_start, HEAP_CAPACITY);
    return gc;
}

void Garbage_Collector::gc_collect()
{
    out << "-------- Called GC Collect --------" << LBR;
    log_info();

    get_roots();
    
    unmark_chunks();
    
    mark_phase();

    sweep_phase();
    
}

void Garbage_Collector::add_gc_roots(void** root)
{
    if (potential_roots_size >= MAX_ARRAY_CAP) {
        gc_collect();
        if (potential_roots_size >= MAX_ARRAY_CAP) {
            out << "Reached Potential Nodes Limit" << LBR;
            log_info();
            return;
        }
    }

    out << "Called add_gc_roots for root -> " << root << LBR;
    
    if (root != NULL && is_pointer_within_heap(*root)) {
        out << "Inserting root " << root << " in potential_roots_list" << LBR;
        log_info();
        potential_stack_vars_containing_roots_list[potential_roots_size] = root;
        potential_roots_size++;
    }
}

void Garbage_Collector::gc_dump()
{
    if (DEBUG_MODE == false) return;

    out << "----- GC DUMP -----" << LBR;
    log_info();

    out << "---- Potential Root Chunk List ----" << LBR;
    log_info();
    for (int i = 0; i < potential_roots_size; i++) {
        out << i << ". " << potential_stack_vars_containing_roots_list[i] << LBR;
        log_info();
    }
    out << "Potential Root Chunk List Size = " << potential_roots_size << LBR;
    log_info();

    out << "--- Root chunk list ----" << LBR;
    log_info();
    for (int i = 0; i < root_chunk_list_size; i++) {
        out << i << ". " << root_chunk_list[i] << LBR;
        log_info();
    }

    out << "Root Chunk List Size = " << root_chunk_list_size << LBR;
    log_info();
}

void Garbage_Collector::mark_phase()
{
    out << "Starting marking phase.." << LBR;
    log_info();
    // Use the root_chunk_list as stack
    if (root_chunk_list_size == 0) {
        out << "Nothing to mark" << LBR;
        log_info();
        return;
    }

    while (root_chunk_list_size > 0) {
        root_chunk_list_size--;
        Chunk_Metadata* top = reinterpret_cast<Chunk_Metadata*>(root_chunk_list[root_chunk_list_size]);

        // Find pointers (chunk_ptrs) inside the current chunk and add them to the root list
        // This expands the stack with new potential chunks to be marked
        find_chunks_within_chunk(top);
        
        // Mark the chunk
        top->gc_mark = true;
        
        out << "------------ CHUNK : " << top << " -> " << top->currentChunk() << " MARKED ------------";
        log_info();

    }

}
