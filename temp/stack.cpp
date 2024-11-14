#include <iostream>
using namespace std;

int main() {


    void* stack_front;
    asm("movq %%rsp, %0" : "=r"(stack_front));
    
    int temp1 = 1;
    int temp2 = 2;
    int temp3 = 3;
    
        
    
    cout<<reinterpret_cast<char*>(&temp1) - reinterpret_cast<char*>(stack_front)<<endl;
    cout<<reinterpret_cast<char*>(&temp2) - reinterpret_cast<char*>(stack_front)<<endl;
    cout<<reinterpret_cast<char*>(&temp3) - reinterpret_cast<char*>(stack_front)<<endl;
    
    
    return 0;
}
