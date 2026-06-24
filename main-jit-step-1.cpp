/*
Project description.
====================

Overall goal: JIT and run  assembler fragment in figure (A) on x86-64 (post 2013 architectures, i.e. Core iX 6XXX and better). 
Use only the Linux API - assure POSIX compliance whenever possible, prefer C++/C standard library fuctions over OS if feasible. 
Use the official Intel/AMD Manuals, the relevant System V ABI and man pages, an occasional peek into https://cppreference.com/ is also allowed. 

!! NEVER apply AI !!

References: 
 - Intel 64 and IA-32 Architectures Software Developer's Manual (June 2026 version), relevant here is especially the 
 - https://wiki.osdev.org/System_V_ABI (System V ABI Application Binary Interface)
 - 

(A) The assembler fragment to be compiled and its semantics.
============================================================
kind Event;

OblectamentaDataLabel counter;                         // section .dara
oblectamenta{global{data{counter;0;};};};              // counter dd 0

sm{
 S;
 states{Initial;Final;};
 t{Initial;Final;doIncrementCounter;};
 Actions{
  doIncrementCounter{
   oblectamenta{text{asm{ 

    OblectamentaCodeLabel start_loop, end_loop;       // labels 
    val i = R0;                                       // use 'i' to reference 64 bit wide register R0
    ldi32(counter);                                   // Put value at location counter on the compute stack
    sti32(i);                                         // Store 32 bit wide value at top of compute stack to i aka R0
                                                      // ==> MOV R0,counter
    start_loop;
    ldi32(i);     
    ldi32(10000000);
    blteq(end_loop);
    ldi32(i);
    ldi32(1);
    addi32;
    sti32(i);    
    buc(start_loop);
    end_loop;


   };};};
 };
 };
};

Simulation{
 Start{S;};
};


How to build and run
====================
c++ -std=C++2x -Wall -Wextra -fPIC main-jit-step-1.cpp -o main-jit-step-1 && ./main-jit-step-1

*/



// Step #1: request a chunk of memory big enough to hold a RET instruction, i.e. 1 byte, which is executable.
//          Write a ret opcode at the very start of the chunk, jump to the instruction. 
//          Expectation: Successful return from the jump
#include <iostream>
#include <sys/mman.h>


using gen_ret_t = int;

namespace x86_64{
 gen_ret_t opcode_ret(char* text){ 
    *text = 0xc3; return 1; 
 }
}


void create_ret_fragment_and_execute(){
    using namespace x86_64;
    void * code_frag_raw =  mmap(0, 128, PROT_READ | PROT_WRITE | PROT_EXEC,  MAP_PRIVATE | MAP_ANONYMOUS, -1,0) ;
    // call memory map sys service, request 128 byte of writable, readable, executable private shared, anonymous, not file
    // backed memory
    if (code_frag_raw == MAP_FAILED){
        std::cerr << "***Error: mmap()\n";return;
    }

    auto code_frag = (char *)code_frag_raw;
    
    opcode_ret(code_frag); // write ret opcode

    std::cerr << "Jump in\n";
    // We have to cast code_frag to void(*)(), i.e. function pointer to void(), and call it: ( (void(*)() )(code_frag))()
    ((void(*)())(code_frag))(); // Jump to code
    std::cerr << "Jump out\n";   
}

int main([[maybe_unused]]int argc, [[maybe_unused]]char ** argv){
    create_ret_fragment_and_execute();
    return 0;
}