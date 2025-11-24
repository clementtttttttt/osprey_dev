#include "CPU6809.h"
#include <iostream>


CPU6809::CPU6809(uint8_t(*rmf)(uint16_t addr),  void(*wmf)(uint16_t addr,uint8_t byte))
{
    this->read_mem = rmf;
    this->write_mem = wmf;
    reset = true;
    //ctor
}

CPU6809::~CPU6809()
{
    //dtor
}


uint16_t CPU6809::dir_get_addr(){
    uint16_t addr = read_mem(regs_16[reg_pc]++); //low 8 bits of addr from postbyte

    addr |= (*regs_8[reg_dp]) << 8;

    return addr;
}

void CPU6809::op_com_dir(){
    uint16_t addr = dir_get_addr();
    int8_t mem_byte = read_mem(addr);
    mem_byte = ~mem_byte;
    write_mem(addr, mem_byte);

    cc &= ~(0b1100); //unset zero and neg
    if(mem_byte & 0x80) cc |= 0b1000; //negative flag
    if(mem_byte == 0) cc |= 0b100; //zero

    cc &= ~(0b10); //v flag laways cleared
    cc |= 0b1;
}

void CPU6809::op_neg_dir(){

    uint16_t addr = dir_get_addr();

    int8_t mem_byte = read_mem(addr);

    cc |= 1; //set carry
    cc &= 0b10; //unset overlfow
    if(mem_byte == 0x80) cc |= 0b10; //set overflow if = 0x80
    if(mem_byte == 0) cc &= ~(1); //unset carry if original is 0

    mem_byte = -mem_byte;

    cc &= ~(0x8 | 0x4); //unset neg
    if(mem_byte & 0x80){
        cc |= 0x8;
    }
    if(mem_byte == 0){
        cc |= 0x4;
    }


    write_mem(addr, mem_byte);


}

void CPU6809::op_asr_dir(){

    uint16_t addr = dir_get_addr();

    int8_t mem_byte = read_mem(addr);

    cc &= ~(0b1);
    if(mem_byte & 1) cc |= 0b1; //bottom bit shifted in carry

    mem_byte >>= 1;
    write_mem(addr, mem_byte);

    cc &= ~(0b1100); //neg and zflag cleared
    if(mem_byte == 0) cc |= 0b100; //set z flag if zero
    if(mem_byte & 0x80) cc |= 0b1000; //set n flag if msb set

}

void CPU6809::op_lsr_dir(){

    uint16_t addr = dir_get_addr();

    uint8_t mem_byte = read_mem(addr);

    cc &= ~(0b1);
    if(mem_byte & 1) cc |= 0b1; //bottom bit shifted in carry

    mem_byte >>= 1;
    write_mem(addr, mem_byte);

    cc &= ~(0b1100); //neg and zflag cleared
    if(mem_byte == 0) cc |= 0b100; //set z flag if zero

}

void CPU6809::op_lsl_dir(){

    uint16_t addr = dir_get_addr();

    uint8_t mem_byte = read_mem(addr);

    cc &= ~(0b1);
    if(mem_byte & 0x80) cc |= 0b1; //top bit shifted into carry
    cc &= ~(0b10); //clear overflow
    if(!!(mem_byte & 0x40) ^ !!(mem_byte & 0x80))cc |= 0b10; //set overflow to xor of bit 6&7

    mem_byte <<= 1;
    write_mem(addr, mem_byte);

    cc &= ~(0b1100); //neg and zflag cleared
    if(mem_byte == 0) cc |= 0b100; //set z flag if zero

}

void CPU6809::op_ror_dir(){
    uint16_t addr = dir_get_addr();
    uint8_t mem_byte = read_mem(addr);
    uint8_t ror_c_flag = mem_byte & 1; //lsb shifted to c flag
    mem_byte >>= 1;

    if(cc & 1) mem_byte |= 0x80; //old c flag shifted to msb

    cc &= 0xfe; //clear c flag so we can OR it again
    cc |= ror_c_flag;

    set_nz(mem_byte);

    write_mem(addr, mem_byte);
}


void CPU6809::set_nz(uint8_t mem_byte){
    cc &= ~(0b1100); //neg & z flag cleared
    if(mem_byte & 0x80) cc |= 0b1000; //neg set if bit 7 on
    if(mem_byte == 0) cc |= 0b100; //z if zero
}

void CPU6809::op_rol_dir(){
    uint16_t addr = dir_get_addr();
    uint8_t mem_byte = read_mem(addr);
    uint8_t rol_c_flag = !!(mem_byte & 0x80); //lsb shifted to c flag
    mem_byte <<= 1;

    if(cc & 1) mem_byte |= 1; //old c flag shifted to msb

    cc &= 0xfe; //clear c flag so we can OR it again
    cc |= rol_c_flag;
    cc &= ~(0b10); //clear overflow
    if(!!(mem_byte & 0x40) ^ !!(mem_byte & 0x80))cc |= 0b10; //set overflow to xor of bit 6&7

    write_mem(addr, mem_byte);

    set_nz(mem_byte);
}

void CPU6809::op_inc_dir(){
    uint16_t addr = dir_get_addr();
    uint8_t mem_byte = read_mem(addr);

    cc &= ~(0b10); //overflow unset
    if(mem_byte == 0x7f) cc |= 0b10; //overflow set if old value = 0x80

    write_mem(addr, ++mem_byte);
    set_nz(mem_byte);

}

void CPU6809::op_tst_dir(){
    uint16_t addr = dir_get_addr();
    uint8_t mem_byte = read_mem(addr);

    cc &= ~(0b10); //overflow unset
    set_nz(mem_byte);

}

void CPU6809::op_dec_dir(){
    uint16_t addr = dir_get_addr();
    uint8_t mem_byte = read_mem(addr);

    cc &= ~(0b10); //overflow unset
    if(mem_byte == 0x80) cc |= 0b10; //overflow set if old value = 0x80

    write_mem(addr, --mem_byte);
    set_nz(mem_byte);

}


void CPU6809::op_clr_dir(){
    uint16_t addr = dir_get_addr();
    write_mem(addr, 0); //clears memory byte
    cc &= ~(0b1011); //clear negative, overflow & carry
    cc |= 0b100; //set zero flag

}

void CPU6809::op_jmp_dir(){
    uint16_t addr = dir_get_addr();
    regs_16[reg_pc] = addr; //jmp just sets pc

}

void CPU6809::page_2(){

}

void CPU6809::page_3(){

}

void CPU6809::op_nop(){

}


void CPU6809::op_sync(){
    //TODO: SYNC

}

uint16_t CPU6809::read_16be(uint16_t addr){
    return (read_mem(addr) << 8) | read_mem(addr+1);
}

void CPU6809::write_16be(uint16_t addr, uint16_t dat){
    write_mem(addr, dat >> 8);
    write_mem(addr, dat);
}

void CPU6809::op_lbra(){
    int16_t off = read_16be(regs_16[reg_pc]);
    regs_16[reg_pc] += off;

}

void CPU6809::op_lbsr(){
    int16_t off = read_16be(regs_16[reg_pc]);

    uint16_t ret_addr = regs_16[reg_pc] + 2;
    write_16be(regs_16[reg_s] -= 2, ret_addr); //stack push return

    regs_16[reg_pc] += off;
}

void CPU6809::op_orcc(){
    uint8_t imm = read_mem(regs_16[reg_pc]++); //read then increment pc
    cc |= imm;

}

void CPU6809::op_andcc(){
    uint8_t imm = read_mem(regs_16[reg_pc]++); //read then increment pc
    cc &= imm;

}


void CPU6809::op_exg(){ //6809 IMPLEMENTATIon
    uint8_t imm = read_mem(regs_16[reg_pc]++); //read then increment pc
    uint8_t r0_code = imm >> 4;
    uint8_t r1_code = imm & 0xf;

    uint16_t r0_contents = 0xffff;
    uint16_t r1_contents = 0xffff;

    if((r0_code & 0b1000) && (r1_code & 0b1000)){
        r0_contents = *regs_8[r0_code & 0b111];
        *regs_8[r0_code & 0b111] = *regs_8[r1_code & 0b111];
        *regs_8[r1_code & 0b111] = r0_contents; //8 bit swap
    }

    else if(!(r0_code & 0b1000) && !(r1_code & 0b1000)){
        //16 bit swap
        r0_contents = regs_16[r0_code];
        regs_16[r0_code] = regs_16[r1_code];
        regs_16[r1_code] = r0_contents;
    }
    else if((r0_code & 0b1000) && !(r1_code & 0b1000)){
        //8 to 16 swap
        r0_contents = *regs_8[r0_code & 0b111];
        *regs_8[r0_code & 0b111] = regs_16[r1_code];
        regs_16[r1_code] = 0xff00 | r0_contents;
    }
    else if((r1_code & 0b1000) && !(r0_code & 0b1000)){
        //16 to 8 swap
        r1_contents = *regs_8[r1_code & 0b111];
        *regs_8[r1_code & 0b111] = regs_16[r0_code];
        regs_16[r0_code] = 0xff00 | r1_contents;
    }
}


void CPU6809::op_tfr(){ //6809 IMPLEMENTATIon
    uint8_t imm = read_mem(regs_16[reg_pc]++); //read then increment pc
    uint8_t r0_code = imm >> 4;
    uint8_t r1_code = imm & 0xf;

    uint16_t r0_contents = 0xffff;
    uint16_t r1_contents = 0xffff;

    if((r0_code & 0b1000) && (r1_code & 0b1000)){
        *regs_8[r1_code & 0b111] = *regs_8[r0_code & 0b111]; //8 bit
    }

    else if(!(r0_code & 0b1000) && !(r1_code & 0b1000)){
        //16 bit
        regs_16[r1_code] = regs_16[r0_code];
    }
    else if((r0_code & 0b1000) && !(r1_code & 0b1000)){
        //8 to 16
        r0_contents = *regs_8[r0_code & 0b111];
        regs_16[r1_code] = 0xff00 | r0_contents;
    }
    else if((r1_code & 0b1000) && !(r0_code & 0b1000)){
        //16 to 8
                *regs_8[r1_code & 0b111] = regs_16[r0_code];
    }
}


void CPU6809::op_sex(){

    set_nz(*regs_8[reg_b]);
    if(*regs_8[reg_b] & 0x80) *regs_8[reg_a] = 0xff;
    else *regs_8[reg_a] = 0;

}

void CPU6809::op_daa(){
    uint8_t acc_val = *regs_8[reg_a];

    uint8_t upper_nibble = acc_val >> 4;
    uint8_t lower_nibble = acc_val & 0b1111;
    uint8_t old_upper_nibble = upper_nibble; //for c flag

    if((cc & CC_C)
    || upper_nibble > 9
    || (upper_nibble > 8 && lower_nibble > 9)){
            upper_nibble += 6;
    }
    if((cc & CC_H) || lower_nibble > 9){
        lower_nibble += 6;
    }

    cc &= ~(CC_C); //clear
    if(old_upper_nibble > 9) cc |= CC_C; //if bigger than 9 then set

    upper_nibble &= 0xf;
    lower_nibble &= 0xf;
    *regs_8[reg_a] = (upper_nibble < 4) | lower_nibble;

    set_nz(*regs_8[reg_a]);
}

void (CPU6809::* CPU6809::opcode_table[MAX_OPCODE])() = {
    &CPU6809::op_neg_dir, nullptr,nullptr, &CPU6809::op_com_dir, &CPU6809::op_lsr_dir, nullptr, &CPU6809::op_ror_dir, &CPU6809::op_asr_dir, &CPU6809::op_lsl_dir, &CPU6809::op_rol_dir, &CPU6809::op_dec_dir, nullptr, &CPU6809::op_inc_dir, &CPU6809::op_tst_dir, &CPU6809::op_jmp_dir, &CPU6809::op_clr_dir,
    &CPU6809::page_2, &CPU6809::page_3, &CPU6809::op_nop, &CPU6809::op_sync, nullptr, nullptr, &CPU6809::op_lbra, &CPU6809::op_lbsr, nullptr, &CPU6809::op_daa, &CPU6809::op_orcc, nullptr, &CPU6809::op_andcc, &CPU6809::op_sex, &CPU6809::op_exg, &CPU6809::op_tfr,

};

#include <unistd.h>

void CPU6809::run_cycles(uint32_t c){
        if(reset){
            const static int RESET_VEC_HIGH = 0xfffe;
            const static int RESET_VEC_LOW = 0xffff;

            uint16_t reset_addr;
            reset_addr = (read_mem(RESET_VEC_HIGH) << 8) | read_mem(RESET_VEC_LOW);

            regs_16[reg_pc] = reset_addr;
            reset = false;
        }
        else{

            uint8_t IR = read_mem(regs_16[reg_pc]++);
            void(CPU6809::* func)() = opcode_table[IR];
            if(func == nullptr){

            }
            else{
                (this->*func)();
            }
        }


}
