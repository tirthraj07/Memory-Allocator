#ifndef CHUNK_METADATA_H
#define CHUNK_METADATA_H
#pragma once

#include <cstddef>
#include <iostream>
/**
 * @class Chunk_Metadata
 * @brief Holds metadata for each memory chunk in the heap.
 *
 * The metadata for each chunk includes its size, allocation status, and pointers to
 * the previous and next chunks in a doubly linked list.
 */
class Chunk_Metadata {
public:
    std::size_t chunk_size;         ///< Size of the current chunk (excluding metadata)
    bool is_free;                   ///< Flag to indicate if the chunk is free or not
    Chunk_Metadata* prev;           ///< Pointer to the previous chunk in the list
    Chunk_Metadata* next;           ///< Pointer to the next chunk in the list
    bool gc_mark;

    /**
     * @brief Constructs a Chunk_Metadata object with the specified size and allocation status.
     * @param chunk_size Size of the memory chunk (excluding metadata).
     * @param is_free Boolean flag indicating if the chunk is free or allocated.
     */
    Chunk_Metadata(std::size_t chunk_size, bool is_free)
        : chunk_size(chunk_size), is_free(is_free), prev(nullptr), next(nullptr), gc_mark(!is_free) {}

    /**
     * @brief Retrieves a pointer to the data area of the current chunk, immediately following its metadata.
     * @return Pointer to the data area of the chunk.
     */
    void* currentChunk();
};


#endif