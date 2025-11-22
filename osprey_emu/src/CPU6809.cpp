#include "CPU6809.h"

CPU6809::CPU6809()
{
    //ctor
}

CPU6809::~CPU6809()
{
    //dtor
}


void (*opcode_table[UCHAR_MAX])();

void CPU6809::run_cycles(uint32_t c){
    for(int cycles = 0; cycles < c; ++c){
        uint8_t IR = read_mem(regs_16[reg_pc]);

        ++regs_16[reg_pc];

        (*opcode_table[IR])();


    }

}
