#include <iostream>

int main() {
    #if defined(__x86_64__) || defined(_M_X64)
        std::cout << "64-bit architecture" << std::endl;
    #elif defined(__i386) || defined(_M_IX86)
        std::cout << "32-bit architecture" << std::endl;
    #else
        std::cout << "Unknown architecture" << std::endl;
    #endif
    return 0;
}
