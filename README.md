# A just-in-time compiler from scratch
## Constraints:
 - Needs only a c++ compiler to build
 - No exrernal dependencies.
 - Only Linux API, POSIX compliant whenever possible.
 - C++/C standard library fuctions preferred over OS if feasible. 
 - Code references official Intel/AMD Manuals, and the relevant System V ABI
 - **No AI**
 - No **PACKAGE MANAGER, cmake, bundlemumble etc.** needed.

References: 
 [1] Intel 64 and IA-32 Architectures Software Developer's Manual ([June 2026 version](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)) 
 [2] https://wiki.osdev.org/System_V_ABI (System V ABI Application Binary Interface)

## First Step
### Challenge: Write a just-in-time compiler for a reasonable subset of an experimental language (https://github.com/cepsdev/machines4ceps/blob/master/core/include/vm/vm_base.hpp) which runs on x64 Linux 
### First goal: JIT and run assembler fragment in figure (A) on x86-64 (post 2013 architectures, i.e. Core iX 6XXX and better). 
## (A) The fragment to be compiled and its semantics.
### (A.1) The assembler fragment
```javascript
kind Event;

OblectamentaDataLabel counter;                         // section .data
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
```
### (A.2) Its meaning in terms of a semantically equivalent Python program
```python
counter:int = 0
while counter <  10000000:
    counter = counter + 1
```







 };
};
