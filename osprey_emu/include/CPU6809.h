#ifndef CPU6809_H
#define CPU6809_H

#include <cstdint>
#include <climits>
class CPU6809
{
    public:
        CPU6809(uint8_t(*rmf)(uint16_t addr),  void(*wmf)(uint16_t addr,uint8_t byte));
        virtual ~CPU6809();

        void run_cycles(uint32_t c);

    protected:

    private:

        static const int MAX_OPCODE = 255;

        uint8_t(*read_mem)(uint16_t addr);
        void(*write_mem)(uint16_t addr,uint8_t byte);

        uint16_t dir_get_addr();
        static void (CPU6809::*  opcode_table[MAX_OPCODE])();

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

        enum cc_flags{
            CC_C = 0b1,
            CC_V = 0b10,
            CC_Z = 0b100,
            CC_N = 0b1000,
            CC_I = 0b10000,
            CC_H = 0b100000,
            CC_F = 0b1000000,
            CC_E = 0b10000000
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



        void op_neg_dir();
        void op_com_dir();
        void op_lsr_dir();
        void op_ror_dir();
        void op_asr_dir();
        void op_lsl_dir();
        void op_rol_dir();
        void op_dec_dir();
        void op_inc_dir();
        void op_tst_dir();
        void op_jmp_dir();
        void op_clr_dir();
        void op_daa();
        void page_2();
        void page_3();
        void op_nop();
        void op_sync();
        void op_lbra();
        void op_lbsr();
        void op_orcc();
        void op_andcc();
        void op_sex();
        void op_exg();
        void op_tfr(); //6809 IMPLEMENTATIon

        uint16_t read_16be(uint16_t addr);
        void write_16be(uint16_t addr, uint16_t dat);

        void set_nz(uint8_t mem_byte);

        bool reset;
};

#endif // CPU6809_H
