#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

// Example heap bounds
uintptr_t heap_start = 0x600000;  // Example heap start address
uintptr_t heap_end   = 0x900000;  // Example heap end address

bool is_pointer_in_heap(uintptr_t ptr) {
    return ptr >= heap_start && ptr < heap_end;
}

void find_roots_from_stack() {
    uintptr_t stack_bottom;
    uintptr_t rsp;

    // Get current stack pointer
    asm("mov %%rsp, %0" : "=r"(rsp));

    // Get the stack base for the current thread
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);
    void* stack_addr;
    size_t stack_size;
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);

    stack_bottom = (uintptr_t)stack_addr + stack_size;

    std::cout << "Scanning stack from " << rsp << " to " << stack_bottom << std::endl;

    for (uintptr_t* p = (uintptr_t*)rsp; p < (uintptr_t*)stack_bottom; ++p) {
        uintptr_t potential_ptr = *p;
        if (is_pointer_in_heap(potential_ptr)) {
            std::cout << "Root found: " << (void*)potential_ptr << std::endl;
        }
    }
}

int main() {
    // Example use
    uintptr_t my_variable_in_heap = heap_start + 128; // Simulate heap allocation
    find_roots_from_stack();
    return 0;
}

