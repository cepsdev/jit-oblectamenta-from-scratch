// THIS IS STEP #2
// Implement support of MOV REG1, REG2; MOV REG, IMM; MOV REG, MEMORY.


/*
Project description.
====================

Overall goal: JIT and run  assembler fragment in figure (A) on x86-64 (post 2013 architectures, i.e. Core iX 6XXX and better). 
Use only the Linux API - assure POSIX compliance whenever possible, prefer C++/C standard library fuctions over OS if feasible. 
Use the official Intel/AMD Manuals, the relevant System V ABI and man pages, an occasional peek into https://cppreference.com/ is also allowed. 

!! NEVER apply AI !!

References: 
 [1] Intel 64 and IA-32 Architectures Software Developer's Manual (June 2026 version), relevant here is especially the 
 [2] https://wiki.osdev.org/System_V_ABI (System V ABI Application Binary Interface)


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
c++ -std=C++2x -Wall -Wextra -fPIC main-jit-step-2.cpp -o main-jit-step-2 && ./main-jit-step-2

*/



// Step #2: Implement support of MOV REG1, REG2; MOV REG, IMM; MOV REG, MEMORY.

// Step #2.1: MOV DEST_REG, SOURCE_REG
// How to test? We take advantage of the Sys V ABI, i.e. the way arguments and return values are passed on Linux.
// The register RDI gets the first argument of a function call
// The register RAX carries the result of a function
// ==> Step 2.1 MOV RAX, RDI
//     This way we can check that our implementation is correct by comparing the returned result with the input:
//     input == f(input), where f jumps directly into the generated code
// Turns out that things are more complicated, by running 
// uint64_t x86_64::jump_to_program(char* text_seg, uint64_t arg){
//    // We have to cast code_frag to void(*)(), i.e. function pointer to void(), and call it: ( (void(*)() )(code_frag))()
//    return ((uint64_t(*)(uint64_t))(text_seg))(arg);
// }
//
// we get every time: r == in if we run autro r =jump_to_program(code, in ); 
// By looking at the assembler output, it becomes clear why, %rax is equal %rdi before the function call, the reason 
// for that is the handling of the input parameters goin into the call, we have to make some tweaks.
// 
//
// 
// _ZN6x86_6415jump_to_programEPcm:
// .LFB2359:
// 	.loc 1 101 63
// 	.cfi_startproc
// 	endbr64
// 	pushq	%rbp
// 	.cfi_def_cfa_offset 16
// 	.cfi_offset 6, -16
// 	movq	%rsp, %rbp
// 	.cfi_def_cfa_register 6
// 	subq	$16, %rsp
// 	movq	%rdi, -8(%rbp)
// 	movq	%rsi, -16(%rbp)
// 	.loc 1 103 13
// 	movq	-8(%rbp), %rdx
// 	.loc 1 103 47
// 	movq	-16(%rbp), %rax
// 	movq	%rax, %rdi
// 	call	*%rdx
// Ok, tweaking done, we settle for MOV RAX,RSI // RSI being the second argument
#include <iostream>
#include <sys/mman.h>
#include <cstdint>

using gen_ret_t = int;

namespace x86_64{
 gen_ret_t opcode_ret(char* text){
    // RET - Return From Procedure.
    // Near return to calling procedure ([1] Vol. 2B Chapter 4, p.569)
    *text = 0xc3; return 1; 
 }

 gen_ret_t opcode_mov_dest_reg_source_reg(char* text, int dest, int source){
    // MOV dest, source 
    // REX.W + 89 /r | MOV r/m64,r64
    /*
    Figure 2-2. Table Interpretation of Mod/RM Byte
    Mod MM
    RM       mmm
    REG   RRR
       MMRRRmmm
    [1] 2-4 Vol 2A
    */
    text[0] = 0x48; // REX.W
    text[1] = 0x89; // Mod | Reg | R/M
    text[2] = 0xF0; // Mod = 11 , Reg = 110, R/M = 000 | 11110000
    return 3; 
 }

 void jump_to_program(char* );
 uint64_t jump_to_program(char* , uint64_t);
}

void x86_64::jump_to_program(char* text_seg){
    // We have to cast code_frag to void(*)(), i.e. function pointer to void(), and call it: ( (void(*)() )(code_frag))()
    ((void(*)())(text_seg))();
}

uint64_t x86_64::jump_to_program(char* text_seg, uint64_t arg){
    auto arg1 = 0;
    auto arg2 = arg;
    // This should do it rax == arg1 == 0 before the call, hence arg != return value for arg != 0
    return ((uint64_t(*)(uint64_t,uint64_t))(text_seg))(arg1,arg2);
}

void create_ret_fragment_and_execute(bool debug_info = true){
    using namespace x86_64;
    void * code_frag_raw =  mmap(0, 128, PROT_READ | PROT_WRITE | PROT_EXEC,  MAP_PRIVATE | MAP_ANONYMOUS, -1,0) ;
    // call memory map sys service, request 128 byte of writable, readable, executable private shared, anonymous, not file
    // backed memory
    if (code_frag_raw == MAP_FAILED){
        std::cerr << "***Error: mmap()\n";return;
    }

    auto code_frag = (char *)code_frag_raw;
    auto loc{0};
    loc = opcode_mov_dest_reg_source_reg(code_frag + loc, 0, 0);

    
    opcode_ret(code_frag + loc); // write ret opcode
    uint64_t arg{43};
    if(debug_info) std::cerr << "Jump in, input vaue = "<< arg <<"\n";
    auto r = jump_to_program(code_frag,arg);
    if(debug_info)std::cerr << "Jump out, return value = "<<r<<"\n";   
}

int main([[maybe_unused]]int argc, [[maybe_unused]]char ** argv){
    constexpr auto print_debug_messages{true};
    create_ret_fragment_and_execute(print_debug_messages);
    return 0;
}