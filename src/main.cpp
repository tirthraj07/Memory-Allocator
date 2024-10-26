#include <iostream>
#include "allocator.h"

int main(){
    std::cout<<"Starting main.cpp.. "<<std::endl;
    Allocator alloc;

    char* ptr1 = (char*)alloc.allocate(80);
    char* ptr2 = (char*)alloc.allocate(5);



    std::cout << "\nAdding ptr1 and ptr2" << std::endl;
    alloc.heap_dump();

    alloc.deallocate(ptr1);

    std::cout << "\nAfter deleting ptr1" << std::endl;
    alloc.heap_dump();


    char* ptr3 = (char*)alloc.allocate(5);
    std::cout << "\nAfter adding ptr3" << std::endl;
    alloc.heap_dump();


    alloc.deallocate(ptr2);
    std::cout << "\nAfter deleting ptr2" << std::endl;
    alloc.heap_dump();

    
}