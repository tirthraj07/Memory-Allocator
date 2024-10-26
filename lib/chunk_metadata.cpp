#include "chunk_metadata.h"

void* Chunk_Metadata::currentChunk() {
    return reinterpret_cast<void*>(
        reinterpret_cast<char*>(this) + sizeof(Chunk_Metadata)
        );
}
