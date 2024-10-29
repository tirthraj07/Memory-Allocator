cmake_minimum_required(VERSION 3.10)
project(MemoryAllocator)


set(CMAKE_CXX_STANDARD 17)

include_directories(includes)


add_executable(MemoryAllocator
    src/main.cpp
    "lib/allocator.cpp"
    "lib/chunk_metadata.cpp"  "lib/bst_node.cpp")   

# Instructs the compiler to print as many warnings as possible
# Refer https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html for GCC warning options
target_compile_options(MemoryAllocator PRIVATE -Wall -Wextra -Wpedantic)