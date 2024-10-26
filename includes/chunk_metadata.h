#ifndef CHUNK_METADATA_H
#define CHUNK_METADATA_H

#include <cstddef>

class Chunk_Metadata {
public:
    std::size_t chunk_size;         // Size of the current chunk (excluding metadata)
    bool is_free;                   // Flag to indicate if the chunk is free or not
    Chunk_Metadata* prev;           // Pointer to the previous chunk in the list
    Chunk_Metadata* next;           // Pointer to the next chunk in the list

    Chunk_Metadata(std::size_t chunk_size, bool is_free)
        : chunk_size(chunk_size), is_free(is_free), prev(nullptr), next(nullptr) {}

    // Points to the current chunk's data area (immediately after metadata)
    void* currentChunk();
};


#endif