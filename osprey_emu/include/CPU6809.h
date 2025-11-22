#ifndef CPU6809_H
#define CPU6809_H

#include <cstdint>
#include <climits>
class CPU6809
{
    public:
        CPU6809();
        virtual ~CPU6809();

        void run_cycles(uint32_t c);

    protected:

    private:

        uint8_t(*read_mem)(uint16_t addr);
        void(*write_mem)(uint16_t addr,uint8_t byte);


        enum regs16_t{
            reg_d, reg_x, reg_y,reg_u, reg_s, reg_pc, reg_w, reg_v
        };
        enum regs8_t{
            reg_a,
            reg_b,
            reg_cc,
            reg_dp,
            reg_0_1,
            reg_0_2,
            reg_e,
            reg_f
        };
        uint8_t cc;
        uint8_t dp;
        uint8_t zero;

        uint16_t regs_16[8];
        uint8_t* regs_8[8]{
            /* reg_a*/  reinterpret_cast<uint8_t*>(&regs_16[reg_d])+1,
            /* reg_b*/  reinterpret_cast<uint8_t*>(&regs_16[reg_d]),
            &cc,
            &dp,
            &zero,
            &zero,
            /* reg_e*/  reinterpret_cast<uint8_t*>(&regs_16[reg_w])+1,
            /* reg_f*/  reinterpret_cast<uint8_t*>(&regs_16[reg_w]),


        };

        uint8_t sysram[163840];
        uint8_t rom[32768];



};

#endif // CPU6809_H
