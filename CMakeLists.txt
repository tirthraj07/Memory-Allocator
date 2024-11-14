cmake_minimum_required(VERSION 3.10)
project(MemoryAllocator VERSION 1.0
                        DESCRIPTION "A Custom C++ Memory Allocator"
                        LANGUAGES CXX)
 


set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJECT_NAME}
    src/main.cpp
    "lib/allocator.cpp"
    "lib/chunk_metadata.cpp"  "lib/bst_node.cpp" "lib/garbage_collector.cpp")
    
target_include_directories(${PROJECT_NAME} PRIVATE includes)

# Instructs the compiler to print as many warnings as possible
# Refer https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html for GCC warning options
target_compile_options(MemoryAllocator PRIVATE -Wall -Wextra -Wpedantic)