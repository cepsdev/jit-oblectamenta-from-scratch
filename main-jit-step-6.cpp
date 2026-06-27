//
// THIS IS STEP #6
// Implement AVX Move packed integers, AVX Add packed integers


// How to build and run
// ====================
// c++ -std=C++2x -Wall -Wextra -fPIC main-jit-step-6.cpp -o main-jit-step-6 && ./main-jit-step-6

// Step #6: Implement compares and conditional jumps

#include <iostream>
#include <sys/mman.h>
#include <cstdint>
#include <cassert>

using gen_ret_t = int;

namespace x86_64{
 namespace registers{
  enum {RAX = 0, RCX, RDX, RBX, RSP, RBP, RSI, RDI };
 }
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
    int opcode_reg = source;
    int opcode_rm = dest;
    text[2] = 0xC0 | (opcode_reg << 3) | (opcode_rm) ;
    return 3; 
 }

 gen_ret_t opcode_mov_dest_reg_source_imm(char* text, int dest, uint64_t imm){
    // MOV r64, imm64 
    // REX.W + B8 + rd io | MOV r/m64,r64
    /* [1] Vol 2A 3-2
    +rb, +rw, +rd, +ro — Indicated the lower 3 bits of the opcode byte is used to encode the register operand
    without a modR/M byte. The instruction lists the corresponding hexadecimal value of the opcode byte with low
    3 bits as 000b. In non-64-bit mode, a register code, from 0 through 7, is added to the hexadecimal value of the
    opcode byte. In 64-bit mode, indicates the four bit field of REX.b and opcode[2:0] field encodes the register
     operand of the instruction. “+ro” is applicable only in 64-bit mode. See Table 3-1 for the codes.
    */
    text[0] = 0x48; // REX.W
    text[1] = 0xB8 | dest; // Mod | Reg | R/M
    *(uint64_t*)(text + 2) = imm;
    return 10; 
 }

 gen_ret_t opcode_add_dest_reg_source_reg(char* text, int dest, int source){
    // ADD r/m64, r64 | Add r64 to r/m64
    // REX.W + 01 /r
    if (text != nullptr){
     text[0] = 0x48; // REX.W
     text[1] = 1;
     int opcode_reg = source;
     int opcode_rm = dest;
     text[2] = 0xC0 | (opcode_reg << 3) | (opcode_rm) ;
    }
    return 3; 
 }

 gen_ret_t opcode_unconditional_rip_relative_jump(char* text, int dest){
   //xe9 cd | JMP rel32 | Jump near, relative, RIP = RIP + 32bit, displacement sign extended to 64-bits.
   /*
   cb, cw, cd, cp, co, ct — A 1-byte (cb), 2-byte (cw), 4-byte (cd), 6-byte (cp), 8-byte (co) or 10-byte (ct) value
   following the opcode. This value is used to specify a code offset and possibly a new value for the code segment
   register.
   [1] Vol 2A. 3-504
   */
    text[0] = 0xE9;
    *((uint32_t*)(text +1)) = dest;
    return 5; 
 }

 gen_ret_t opcode_compare(char* text, int reg1, int reg2){
   /*
   Compares the first source operand with the second source operand and sets the status flags in the EFLAGS register
   according to the results.
   */
   //REX.W + 39/r | CMP r/m64, r64 | Compare r64 with r/m64.
   if (text != nullptr){
     text[0] = 0x48; // REX.W
     text[1] = 0x39;
     int opcode_reg = reg1;
     int opcode_rm = reg2;
     text[2] = 0xC0 | (opcode_reg << 3) | (opcode_rm) ;
   }
   return 3; 
 }

 gen_ret_t opcode_jump_if_equal_rip_relative_jump(char* text, int dest){
   //0F 84 cd | JE rel32 | Jump near if equal (ZF = 1)
   // [1] Vol 2A. 3-500
   /*
   cb, cw, cd, cp, co, ct — A 1-byte (cb), 2-byte (cw), 4-byte (cd), 6-byte (cp), 8-byte (co) or 10-byte (ct) value
   following the opcode. This value is used to specify a code offset and possibly a new value for the code segment
   register.
   [1] Vol 2A. 3-504
   */
    text[0] = 0x0F;
    text[1] = 0x84;
    *((uint32_t*)(text +2)) = dest;
    return 6; 
 }

 gen_ret_t opcode_vmovdqa_ymm1_m256(char* text, int dest_ymm_reg, uint64_t mem){
   /*
   [1] Vol2B 4-.59, table row #5   

   VEX.256.66.0F.WIG 6F /r       | Move aligned packed integer values form ymm2/mem to ymm1
   VMOVDQA ymm1,ymm2/m256        |
   
   */
  text[0] = 0xc5;
  return 0;
 }

 int opcode_add_dest_reg_source_reg_size(){
    return  opcode_add_dest_reg_source_reg(nullptr, 0, 0); 
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
    loc += opcode_mov_dest_reg_source_imm(code_frag + loc, registers::RSI, 1);                               // 1: RSI = 1
    loc += opcode_mov_dest_reg_source_imm(code_frag + loc, registers::RDI, 2);                               // 2: RDI = 2
    loc += opcode_compare(code_frag + loc, registers::RDI, registers::RSI );                                 // 3: CMP RDI,RSI
    loc += opcode_jump_if_equal_rip_relative_jump(code_frag + loc,opcode_add_dest_reg_source_reg_size());    // 4: JE 6
    loc += opcode_add_dest_reg_source_reg(code_frag + loc, registers::RSI,registers::RDI);                   // 5: RSI = RSI + RDI
    loc += opcode_mov_dest_reg_source_reg(code_frag + loc, registers::RAX, registers::RSI);                  // 6: RAX = RSI
    opcode_ret(code_frag + loc); // write ret opcode                                                         // 7: RET
    uint64_t arg{43};
    if(debug_info) std::cerr << "Jump in, input vaue = "<< arg <<"\n";
    auto r = jump_to_program(code_frag,arg);
    if(debug_info)std::cerr << "Jump out, return value = "<<r<<"\n";
    // ===> Expected value = 3
    if (r != 3) std::cerr << "***Error: Expected 3\n"; else std::cerr << "Passed\n";
    // No again, but this time we trigger the conditional jump
    loc = 0;
    loc += opcode_mov_dest_reg_source_imm(code_frag + loc, registers::RSI, 1);                               // 1: RSI = 1
    loc += opcode_mov_dest_reg_source_imm(code_frag + loc, registers::RDI, 1);                               // 2: RDI = 1
    loc += opcode_compare(code_frag + loc, registers::RDI, registers::RSI );                                 // 3: CMP RDI,RSI
    loc += opcode_jump_if_equal_rip_relative_jump(code_frag + loc,opcode_add_dest_reg_source_reg_size());    // 4: JE 6
    loc += opcode_add_dest_reg_source_reg(code_frag + loc, registers::RSI,registers::RDI);                   // 5: RSI = RSI + RDI
    loc += opcode_mov_dest_reg_source_reg(code_frag + loc, registers::RAX, registers::RSI);                  // 6: RAX = RSI
    opcode_ret(code_frag + loc); // write ret opcode                                                         // 7: RET
    if(debug_info) std::cerr << "Jump in, input vaue = "<< arg <<"\n";
    r = jump_to_program(code_frag,arg);
    if(debug_info)std::cerr << "Jump out, return value = "<<r<<"\n";
    // ===> Expected value = 1
    if (r != 1) std::cerr << "***Error: Expected 3\n"; else std::cerr << "Passed\n";

}

int main([[maybe_unused]]int argc, [[maybe_unused]]char ** argv){
    constexpr auto print_debug_messages{true};
    create_ret_fragment_and_execute(print_debug_messages);
    return 0;
}