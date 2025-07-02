#include "CPUAtmega16.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
CPUAtmega16::CPUAtmega16()
{
    pc = 0;
    //ctor
}

CPUAtmega16::~CPUAtmega16()
{
    //dtor
}

uint8_t CPUAtmega16::getRegr(uint16_t in){
    return in & 0xf | ((in & 0x200) >> 5);

}

uint8_t CPUAtmega16::getRegd(uint16_t in){
    return (in & 0x1f0) >> 4;

}

uint8_t CPUAtmega16::get4bitRegd(uint16_t in){
    return (in & 0xf0) >> 4 + 16;
}

uint8_t CPUAtmega16::getImmediate8(uint16_t in){
    return in & 0xf | ((in >> 4) & 0xf0);
}


void CPUAtmega16::GRP0(uint16_t in){
    uint8_t subcode = (in >> 8) & 0xf;
    uint8_t rd = (in >> 4) & 0xf;
    uint8_t rr = in & 0xf;
        uint8_t regr_idx = getRegr(in);
    uint8_t regd_idx = getRegd(in);

    switch(subcode){
        case 0:
            //nop
        break;

        case 1: // movw
            regs[rd*2] = regs[rr*2];
            regs[rd*2+1] = regs[rd*2];
        break;
        case 2: //muls
            {
            int16_t result = (int8_t)regs[rd + 16] * (int8_t)regs[rr + 16];
            *reinterpret_cast<int16_t*>(&regs[0]) = result;
            sreg->z = (result == 0);
            sreg->c = !!(result & 0x8000);
            }
        break;
        case 3:
            {
            uint8_t mulsel = (in >> 7) | (in >> 3);
            rd &= 7;
            rr &= 7;
            switch(mulsel){
                case 0:
                { //mulsu
                    int16_t result = (int8_t)regs[rd + 16] * (uint8_t)regs[rr + 16];
                        *reinterpret_cast<int16_t*>(&regs[0]) = result;
                        sreg->z = (result == 0);
                    sreg->c = !!(result & 0x8000);
                }
                break;
                case 1:
                { //fmul
                    uint16_t result = (uint8_t)(regs[rd+16]) * (uint8_t)(regs[rr+16]);
                    sreg->c = !!(result & 0x8000);
                    result <<= 1;
                    sreg->z = (result == 0);
                    *reinterpret_cast<uint16_t*>(&regs[0]) = result;
                }
                break;
                case 2:
                {//fmuls
                    int16_t result = (int8_t)(regs[rd+16]) * (int8_t)(regs[rr+16]);
                    sreg->c = !!(result & 0x8000);
                    result <<= 1;
                    sreg->z = (result == 0);
                    *reinterpret_cast<int16_t*>(&regs[0]) = result;
                }
                break;
                case 3:
                {//fmulsu
                    int16_t result = (int8_t)(regs[rd+16]) * (uint8_t)(regs[rr+16]);
                    sreg->c = !!(result & 0x8000);
                    result <<= 1;
                    sreg->z = (result == 0);
                    *reinterpret_cast<int16_t*>(&regs[0]) = result;
                }

            }
            }
        break;
        case 4:
        case 5:
        case 6:
        case 7:
        //cpc
            setSubSreg(regs[regd_idx], regs[regr_idx], sreg->c);
        break;

        case 8:
        case 9:
        case 0xa:
        case 0xb:
            //sbc
            regs[regd_idx] = setSubSreg(regs[regd_idx], regs[regr_idx], sreg->c);
        break;
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf: // add
            regs[regd_idx] = setAddSreg(regs[regd_idx], regs[regr_idx], 0);
        break;


    }

}

uint8_t CPUAtmega16::isTwoBytes(uint16_t in){
    uint8_t code = in >> 8;
    if(code >= 0x94 && code <= 0x95 && ((in & 0xe) == 0b1100)){ //jmp
        return 1;
    }
    if(code >= 0x90 && code <= 0x93 && !(in & 0xf)){ //sts, lds
        return 1;
    }


    return 0;
}

uint8_t CPUAtmega16::setSubSreg(uint8_t rd, uint8_t rr, uint8_t c){
    int8_t result = (int8_t)rd - (int8_t)rr - c;
    sreg->c = (!(rd & 0x80) && !!(rr & 0x80)) || (!!(rr & 0x80) && !!(result & 0x80)) || (!!(result & 0x80) && !(rd & 0x80));
    sreg->v = (!!(rd & 0x80) && !(rr & 0x80) && !(result & 0x80)) ||
              (!(rd & 0x80) && !!(rr & 0x80) && !!(result & 0x80));
    sreg->n = !!(result & 0x80);
    sreg->s = sreg->n ^ sreg->v;
    sreg->z &= (result == 0);
    sreg->h  = (!(rd & 0x8) && !!(rr & 0x8)) || (!!(rr & 0x8) && !!(result & 0x8)) || (!!(result & 0x8) && !(rd & 0x8));
    return (uint8_t)result;
}
uint8_t CPUAtmega16::setAddSreg(uint8_t rd, uint8_t rr, uint8_t c){
    int16_t result = (int8_t)rd + (int8_t)rr + c;
    sreg->n = !!(result & 0x80);
    sreg->c  = result > 0xff;
    sreg->z = (result == 0);
    sreg->v = (!!(rd & 0x80) && !!(rr & 0x80) && !(result & 0x80)) || (!(rd & 0x80) && !(rr & 0x80) && !!(result & 0x80));
    sreg->s = sreg->n ^ sreg->v;
    sreg->h = ((rr & 0xf) + (rd & 0xf)) > 0xf;
    return (uint8_t) result;
}


void CPUAtmega16::setBitopSreg(uint8_t in){
    sreg->v = 0;
    sreg->n = !!(in & 0x80);
    sreg->s = sreg->n;
    sreg->z = (in == 0);
}
//ovid
void CPUAtmega16::skipNextInstruction(){
    ++pc;
    if(isTwoBytes(flash[pc])) ++pc;


}

void CPUAtmega16::CPI(uint16_t in){

    uint8_t i = getImmediate8(in);
    uint8_t rd = get4bitRegd(in);
    setSubSreg(regs[rd], i, 0);
}

void CPUAtmega16::ANDI(uint16_t in){
    uint8_t i = getImmediate8(in);
    uint8_t rd =get4bitRegd(in);
    regs[rd] = regs[rd] & i;
    setBitopSreg(regs[rd]);
}

void CPUAtmega16::writeData(uint16_t addr, uint8_t dat){
    data[addr] = dat;
}

void CPUAtmega16::writeIO(uint16_t addr, uint8_t dat){
    writeData(addr + 0x20, dat);
}

uint8_t CPUAtmega16::readData(uint16_t addr){
    return data[addr];
}

uint8_t CPUAtmega16::readIO(uint16_t addr){
    return data[addr + 0x20];
}

void CPUAtmega16::ORI(uint16_t in){
    uint8_t i = getImmediate8(in);
    uint8_t rd = get4bitRegd(in);
    regs[rd] = regs[rd] | i;
    setBitopSreg(regs[rd]);
}

void CPUAtmega16::SBCI(uint16_t in){
    uint8_t i = getImmediate8(in);
    uint8_t rd = get4bitRegd(in);
    regs[rd] = setSubSreg(regs[rd], i, sreg->c);
}

void CPUAtmega16::LDI(uint16_t in){
    uint8_t i = getImmediate8(in);
    uint8_t rd = get4bitRegd(in);
    regs[rd] = i;
}

void CPUAtmega16::INOUT(uint16_t in){
    uint8_t rd = getRegd(in);
    uint8_t ioreg = (in & 0b11000000000 >> 5) | (in & 0xf);
    uint8_t is_out = in & 0b100000000000;

    if(is_out){
        writeIO(ioreg,regs[rd]);
    }
    else{
        regs[rd] = readIO(ioreg);
    }

}

void CPUAtmega16::SUBI(uint16_t in){
    uint8_t i = getImmediate8(in);
    uint8_t rd = get4bitRegd(in);
    regs[rd] = setSubSreg(regs[rd], i, 0);
}

void CPUAtmega16::LDD(uint16_t in){ //ldd z+x, y+x, etc
    uint8_t idxreg_idx = !!((in & 0xf)? 28:30);
    uint16_t* idxreg  = reinterpret_cast<uint16_t*>(&regs[idxreg_idx]);
    uint8_t rd = getRegd(in);
    uint8_t offset = (in & 0b111) | ((in >> 7) & 0b11000) | ((in >> 8) & 0b100000);

    if(in & 0b1000000000){
        regs[rd] = readData(*idxreg + offset);

    }
    else{
        writeData(*idxreg + offset,regs[rd]);
    }


}

void CPUAtmega16::push8(uint8_t in){
    data[*sp--] = in;
}

void CPUAtmega16::push16(uint16_t in){
    push8(in & 0xff);
    push8((in >> 8) & 0xff);
}

uint8_t CPUAtmega16::pop8(){
    return data[++*sp];
}

uint16_t CPUAtmega16::pop16(){
    uint16_t ret = pop8();
    ret <<= 8;
    ret |= pop8();
    return ret;
}

void CPUAtmega16::GRP4(uint16_t in){
    if((in & 0b1110) ==  0b1100){ //jmp
        ++pc;
        uint16_t addr = flash[pc];
        pc = addr - 1; //loop round 8192 words (16384k)
        return;
    }

    uint8_t regd = getRegd(in);
    switch((in >> 8) & 0xf){
        case 0:
        case 1:
        case 2:
        case 3:
        {
        uint8_t dir = !!((in >> 8) & 0b10); //true = stroe
        uint8_t idx = in & 0b1111;
        switch(idx){
            case 0:// lds, sts
            {
                ++pc;
                uint16_t addr = flash[pc];
                if(dir){
                    writeData(addr, regs[regd]);
                }
                else{
                    regs[regd] = readData(addr);
                }
            }
            break;
            case 4:
            case 5:
            {
                uint16_t *idxreg = reinterpret_cast<uint16_t*>(&regs[30]);
                uint16_t res = flash[*idxreg >> 1];
                if(*idxreg & 0b1){
                    res >>= 8;
                }
                else res &= 0xff;
                regs[regd] = res;
                if(in & 1) ++*idxreg;


            }
            break;

            case 1:// ldd +X, -X, etc
            case 2:
            case 9:
            case 0xa:
            case 0xd:
            case 0xe:
            {
                uint8_t idxreg_idx = 0;
                switch(in >> 2){
                    case 0: idxreg_idx = 30; break;
                    case 1: idxreg_idx = 28; break;
                    case 2: idxreg_idx = 26; break;
                }
                uint16_t *idxreg = reinterpret_cast<uint16_t*>(&regs[idxreg_idx]);
                uint8_t incdir = in & 1;

                if(dir){
                    if(incdir){
                        data[(*idxreg)++] = regs[regd];
                    }
                    else{
                        data[--(*idxreg)] = regs[regd];
                    }
                }
                else{
                    if(incdir){
                        regs[regd] = data[(*idxreg)++];
                    }
                    else{
                        regs[regd] = data[--(*idxreg)];
                    }
                }
            }
            break;
            case 0xc:
            {
                    uint16_t *idxreg = reinterpret_cast<uint16_t*>(&regs[26]);
                    if(dir){
                        data[*idxreg] = regs[regd];
                    }else{
                        regs[regd] = data[*idxreg];
                    }
            }
            break;
            case 0xf:
            {
                    if(dir){
                        push8(regs[regd]);
                    }
                    else{
                        regs[regd] = pop8();
                    }
            }
            break;

        }
        }
        break;
        case 4://com
            switch(in & 0xf){
                case 0:
                    regs[regd] = ~regs[regd];
                    setBitopSreg(regs[regd]);
                    sreg->c = 1;
                    break;
                case 1:
                    regs[regd] = setSubSreg(0, regs[regd], 0);
                    break;
                case 2:
                    {
                        uint8_t lownib = (regs[regd] & 0xf) << 4;
                        regs[regd] >>= 4;
                        regs[regd] |= lownib;
                    }
                    break;
                case 3:
                { //inc
                    uint8_t c_bac = sreg->c;
                    regs[regd] = setAddSreg(regs[regd], 1, 0);

                    sreg->c = c_bac;
                }
                break;

                case 5: //asr
                    sreg->c = regs[regd] & 1;
                    regs[regd] >>= 1;

                    if(regs[regd] == 0 ) sreg->z = 1;
                    else sreg->z = 0;

                    if(regs[regd] & 0x40){
                        sreg->n = 1;
                        regs[regd] |= 0x80;
                    }
                    else{
                        sreg->n = 0;
                    }
                    sreg->v = sreg->n ^ sreg->c;
                    sreg->s = sreg->n ^ sreg->v;
                break;
                case 6: //lsr
                    sreg->c = regs[regd] & 1;
                    regs[regd] >>= 1;

                    if(regs[regd] == 0 ) sreg->z = 1;
                    else sreg->z = 0;

                    sreg->n = 0;

                    sreg->v = sreg->n ^ sreg->c;
                    sreg->s = sreg->n ^ sreg->v;
                break;
                case 7: //ror
                {
                    uint8_t oldc = sreg->c;
                    sreg->c = regs[regd] & 1;
                    regs[regd] >>= 1;

                    if(oldc){
                        regs[regd] |= 0x80; //rotate
                    }

                    if(regs[regd] == 0 ) sreg->z = 1;
                    else sreg->z = 0;

                    sreg->n = 0;

                    sreg->v = sreg->n ^ sreg->c;
                    sreg->s = sreg->n ^ sreg->v;
                }
                break;
                case 8:
                {
                    if((in >> 8) & 1){
                        //sys instructions group
                        switch(regd & 0xf){
                            case 1: //reti
                                    sreg->i = 1;
                            case 0: //ret
                                    pc = pop16();
                                    --pc;
                            break;
                            case 8:
                                    //TODO: SLEEP
                            break;
                            case 9: //break;
                                    //TODO: fancy pants debugger
                            break;
                            case 0xa:
                                //wdr: TODO: make watchdog
                            break;
                            case 0xc:
                            {
                                uint16_t flashaddr = *reinterpret_cast<uint16_t*>(&regs[30]);
                                uint16_t tmp = flash[flashaddr >> 1];
                                if(flashaddr & 1) tmp >>= 8;
                                regs[0] = tmp & 0xff;
                            }
                            break;
                            case 0xe:
                                //TODO: SPM

                            break;
                        }

                    }
                    else{
                        //flagset instr
                    uint8_t flag_to_set = regd & 0xf;
                    if(!(regd & 0x100)){
                        sreg->val |= (1<<flag_to_set);
                    }
                    else{
                        sreg->val &= ~(1<<flag_to_set);
                    }
                    }
                }
                case 9:
                {

                    if((in >> 8) & 1){
                        //another group
                    }
                    else{
                        //ijmp
                        pc = *reinterpret_cast<uint16_t*>(&regs[30]) - 1;

                    }
                }

                break;
                case 0xa:
                { //dec
                //fixme: does not take into h flag, fix that for other instrs like inc as well
                    uint8_t c_bac = sreg->c;
                    regs[regd] = setSubSreg(regs[regd], 1, 0);

                    sreg->c = c_bac;
                }
                break;

            }
        break;


    }

}

void CPUAtmega16::GRP2(uint16_t in){
    uint8_t regr_idx = getRegr(in);
    uint8_t regd_idx = getRegd(in);
    int GRP2_idx = (in & 0b110000000000) >> 10;
    switch(GRP2_idx){
        case 0: //and
            regs[regd_idx] = regs[regr_idx] & regs[regd_idx];
            setBitopSreg(regs[regd_idx]);
        break;
        case 1: //eor
            regs[regd_idx] = regs[regr_idx] ^ regs[regd_idx];
            setBitopSreg(regs[regd_idx]);
        break;
        case 2: //or
            regs[regd_idx] = regs[regr_idx] | regs[regd_idx];
            setBitopSreg(regs[regd_idx]);
        break;
        case 3: //eor
            regs[regd_idx] = regs[regr_idx] ^ regs[regd_idx];
            setBitopSreg(regs[regd_idx]);
        break;
        case 4: //mov
            regs[regd_idx] = regs[regr_idx];
        break;
    }

}

void CPUAtmega16::RJMP(uint16_t in){
    short off = in & 0xfff;
    if(off & 0x800) off |= 0xf000; // signex
    pc += off;
}

void CPUAtmega16::RCALL(uint16_t in){
    push16(pc + 1);
    RJMP(in);
}

void CPUAtmega16::GRP1(uint16_t in){
    uint8_t regr_idx = getRegr(in);
    uint8_t regd_idx = getRegd(in);

    int GRP1_idx = (in & 0b110000000000) >> 10;

    switch(GRP1_idx){
        case 0: //cpse
            if(regs[regr_idx] == regs[regd_idx]){
                    skipNextInstruction();
            }
        break;

        case 1: // cp
            sreg->z = 1;
            setSubSreg(regs[regd_idx], regs[regr_idx], 0);
        break;

        case 2://sub
            regs[regd_idx] = setSubSreg(regs[regd_idx], regs[regr_idx], 0);
        break;

        case 3: //adc
            regs[regd_idx] = setAddSreg(regs[regd_idx], regs[regr_idx], sreg->c);
        break;

    }




}

short CPUAtmega16::getBranchingOffset(uint16_t in){

    short ret = (in >> 3) & 0x7f;
    if(ret & 0x40){
        ret |= 0xf800;
    }
    return ret;
}

void CPUAtmega16::GRPF(uint16_t in){
    if((in >> 12) & 0x80){

    }
    else{
        //branching instrs
        //TODO: fix this crap
        uint8_t bit = in & 0b111;

        if(sreg->val & (1 << bit)){
            pc += getBranchingOffset(in);

        }
    }

}

void(CPUAtmega16::* CPUAtmega16::opcode_funs[])(uint16_t){
    &CPUAtmega16::GRP0, &CPUAtmega16::GRP1, &CPUAtmega16::GRP2, &CPUAtmega16::CPI,
    &CPUAtmega16::SBCI, &CPUAtmega16::SUBI, &CPUAtmega16::ORI,  &CPUAtmega16::ANDI,
    &CPUAtmega16::LDD, &CPUAtmega16::GRP4, &CPUAtmega16::LDD, &CPUAtmega16::INOUT,
    &CPUAtmega16::RJMP, &CPUAtmega16::RCALL, &CPUAtmega16::LDI, &CPUAtmega16::GRPF
};

void CPUAtmega16::cycle(uint32_t cycles){

    for(uint32_t i=0; i<cycles; ++i){
        std::cout << std::hex <<  "PC = " << pc <<  " 2PC= " << pc * 2 << std::endl;
        uint16_t cir = flash[pc];
        uint8_t opcode = (cir >> 12) & 0xf;
        std::cout << (int)opcode << std::endl;

        (this->*opcode_funs[opcode])(cir);
        pc &= 0x1fff;

        ++pc;

    }
}

void CPUAtmega16::loadHex(std::string fname){

    std::ifstream file;
    file.open(fname);
    try{
        char c;
        std::string buf;
        while(file.get(c)){

            std::getline(file, buf);
            size_t off = buf.find_first_of(":") + 1;
            uint8_t cnt = std::stoul(buf.substr(off, 2), nullptr, 16);
            uint16_t addr = std::stoul(buf.substr(off+2, 4), nullptr, 16) / 2; // in words

            if(buf[off + 2 + 4 + 1] == '1'){
                //eof
                break;
            }

            size_t str_data_off = off + 2 + 4 + 2;

            for(uint8_t i=0;i<cnt;i += 2){ //flash works in words

                uint16_t w = std::stoul(buf.substr(str_data_off, 4), nullptr, 16);
                flash[addr++] = __builtin_bswap16(w);
                str_data_off += 4;
            }
        }

    }catch(int err){
        std::cout << err << std::endl;
    }

}
