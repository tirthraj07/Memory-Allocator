#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <pthread.h>

// Example heap bounds
uintptr_t heap_start = 0x600000;  // Example heap start address
uintptr_t heap_end   = 0x900000;  // Example heap end address

bool is_pointer_in_heap(uintptr_t ptr) {
    return ptr >= heap_start && ptr < heap_end;
}

void find_roots_from_stack() {
    volatile int a = 10;  // Mark as volatile to prevent optimization
    volatile int b = 20;

    uintptr_t stack_bottom;
    uintptr_t start_point;

    // Get the address of a variable close to `a` and `b` for scanning
    start_point = (uintptr_t)&a;

    // Get the stack base for the current thread
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    void* stack_addr;
    size_t stack_size;
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);

    stack_bottom = (uintptr_t)stack_addr + stack_size;

    std::cout << "Scanning stack from " << start_point << " to " << stack_bottom << std::endl;

    for (uintptr_t* p = (uintptr_t*)start_point; p < (uintptr_t*)stack_bottom; ++p) {
        uintptr_t potential_ptr = *p;

        // Check if the current stack address matches `a` or `b`
        if ((uintptr_t)p == (uintptr_t)&a) {
            std::cout << "Variable `a` found at: " << (void*)p << ", Value: " << *(int*)p << std::endl;
        }
        if ((uintptr_t)p == (uintptr_t)&b) {
            std::cout << "Variable `b` found at: " << (void*)p << ", Value: " << *(int*)p << std::endl;
        }

        // Check for heap pointers
        if (is_pointer_in_heap(potential_ptr)) {
            std::cout << "Root found pointing to heap: " << (void*)potential_ptr << std::endl;
        }
    }

    pthread_attr_destroy(&attr);
}

int main() {
    find_roots_from_stack();
    return 0;
}


