#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H
#pragma once

#include "chunk_metadata.h"
#include <sstream>
#include <string>
#include <iostream>

/**
 * @class Garbage_Collector
 * @brief Implements a garbage collector for managing memory within a custom allocator.
 *
 * This class provides functionality to perform garbage collection on a custom heap.
 * It follows a mark-and-sweep algorithm to identify and reclaim unused memory chunks.
 * The garbage collector also supports logging and debugging to help with visualization
 * and testing of the memory management system.
 */
class Garbage_Collector {
public:
    // Deleted copy constructor and assignment operator to enforce singleton behavior.
    Garbage_Collector(const Garbage_Collector&) = delete;
    Garbage_Collector& operator=(const Garbage_Collector&) = delete;

    /**
     * @brief Initiates the garbage collection process.
     * This involves marking reachable chunks (mark phase) and reclaiming unused memory (sweep phase).
     */
    void gc_collect();


    /**
     * @brief Dumps information about the garbage collector's state and the heap layout.
     * Useful for debugging and visualization.
     */
    void gc_dump();


    friend class Allocator;

private:
    std::ostringstream out;                                  ///< Output stream for logging purposes.
    bool DEBUG_MODE;                                         ///< Flag to enable or disable debug logging.

    void** potential_stack_vars_containing_roots_list[1000]; ///< List of potential stack roots (pointers to root variables).
    int potential_roots_size = 0;                            ///< Number of potential roots stored.
                    
    void* root_chunk_list[1000];                             ///< List of identified root memory chunks in the heap.
    int root_chunk_list_size = 0;                            ///< Number of root memory chunks.

    void* heap_start;                                        ///< Pointer to the start of the custom heap.
    std::size_t HEAP_CAPACITY;                               ///< Total capacity of the custom heap in bytes.

    /**
     * @brief Private constructor to enforce singleton pattern.
     * @param debug_mode Whether debug logging is enabled.
     * @param heap_start Pointer to the start of the custom heap.
     * @param HEAP_CAPACITY Total capacity of the heap in bytes.
     */
    Garbage_Collector(bool debug_mode, void* heap_start, size_t HEAP_CAPACITY);

    /**
     * @brief Provides access to the singleton instance of the garbage collector.
     * @param heap_start Pointer to the start of the custom heap.
     * @param HEAP_CAPACITY Total capacity of the heap in bytes.
     * @param debug_mode Whether debug logging is enabled (default is false).
     * @return Reference to the Garbage_Collector instance.
     */
    static Garbage_Collector& getInstance(void* heap_start, size_t HEAP_CAPACITY, bool debug_mode = false);

    /**
     * @brief Logs memory management information if debugging is enabled.
     */
    void log_info();

    /**
     * @brief Checks if a given pointer falls within the bounds of the heap.
     * @param ptr Pointer to check.
     * @return True if the pointer is within the heap; otherwise, false.
     */
    bool is_pointer_within_heap(void* ptr);

    /**
     * @brief Scans for root pointers (global or stack variables) that point to chunks within the heap.
     */
    void get_roots();

    /**
     * @brief Unmarks all chunks in the heap, preparing them for a new garbage collection cycle.
     */
    void unmark_chunks();

    /**
     * @brief Finds all memory chunks referenced within a given chunk and marks them.
     * @param top Pointer to the chunk metadata being inspected.
     */
    void find_chunks_within_chunk(Chunk_Metadata* top);

    /**
     * @brief Performs the sweep phase of garbage collection.
     * Identifies unmarked chunks, reclaims their memory, and merges adjacent free chunks.
     */
    void sweep_phase();

    /**
     * @brief Performs the mark phase of the garbage collection process.
     * Identifies all reachable memory chunks starting from root pointers.
     */
    void mark_phase();

    /**
     * @brief Adds a root pointer to the list of known GC roots.
     * @param root Pointer to the root variable.
     */
    void add_gc_roots(void** root);

};

#endif
