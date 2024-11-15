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
    root_chunk_list_size = 0; // Reset root list size
    for (int i = 0; i < potential_roots_size; i++) {
        void** potential_root = potential_stack_vars_containing_roots_list[i];
        out << i << ". Potential root = " << (void*)potential_root << LBR;
        log_info();
        out << "\t*Potential root = " << *potential_root << LBR;
        log_info();

        // Check if the pointer is within the heap
        if (is_pointer_within_heap(*potential_root)) {
            // Get the chunk associated with the potential root
            Allocator& alloc = Allocator::getInstance(DEBUG_MODE);
            Chunk_Metadata* chunk_ptr = alloc.get_chunk(*potential_root);

            // If the chunk is valid, add it to the root list
            if (chunk_ptr != nullptr) {
                // Store the pointer to the actual data area of the chunk
                root_chunk_list[root_chunk_list_size] = reinterpret_cast<void*>(chunk_ptr);
                root_chunk_list_size++;
            }
        }
    }

    // Debug log: Print root list size and details (optional)
    out << "Root list updated. Total roots: " << root_chunk_list_size << LBR;
    log_info();
}


Garbage_Collector& Garbage_Collector::getInstance(void* heap_start, size_t HEAP_CAPACITY, bool debug_mode)
{
    static Garbage_Collector gc(debug_mode, heap_start, HEAP_CAPACITY);
    return gc;
}

void Garbage_Collector::gc_collect()
{
    out << "Called GC Collect" << LBR;
    log_info();

    get_roots();


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

    out << "--- Root chunk list ----" << LBR;
    log_info();
    for (int i = 0; i < root_chunk_list_size; i++) {
        out << i << ". " << root_chunk_list[i] << LBR;
        log_info();
    }
}
