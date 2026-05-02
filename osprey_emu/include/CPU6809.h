#ifndef CPU6809_H
#define CPU6809_H

#include <cstdint>
#include <climits>

class CPU6809
{
public:
    CPU6809(uint8_t(*rmf)(uint16_t addr), void(*wmf)(uint16_t addr, uint8_t byte));
    virtual ~CPU6809();
    void run_cycles(uint32_t c);

    // Expose flag constants for inline branch helpers
    enum cc_flags {
        CC_C = 0b1,      CC_V = 0b10,     CC_Z = 0b100,    CC_N = 0b1000,
        CC_I = 0b10000,  CC_H = 0b100000, CC_F = 0b1000000, CC_E = 0b10000000
    };

private:
    static const int MAX_OPCODE = 256;

    uint8_t(*read_mem)(uint16_t addr);
    void(*write_mem)(uint16_t addr, uint8_t byte);

    enum regs16_t { reg_d, reg_x, reg_y, reg_u, reg_s, reg_pc, reg_w, reg_v };
    enum regs8_t { reg_a, reg_b, reg_cc, reg_dp, reg_0_1, reg_0_2, reg_e, reg_f };

    uint8_t cc;
    uint8_t dp;
    uint8_t zero;
    uint16_t regs_16[8];
    uint8_t* regs_8[8];
    bool reset;
    uint8_t ir;

    uint16_t read16(uint16_t addr);
    void write16(uint16_t addr, uint16_t val);
    void set_nz8(uint8_t v);
    void set_nz16(uint16_t v);

    void spush(uint8_t v);
    uint8_t spop();
    void upush(uint8_t v);
    uint8_t upop();
    void spush16(uint16_t v);
    uint16_t spop16();
    void upush16(uint16_t v);
    uint16_t upop16();

    uint8_t  ea_imm8();
    uint16_t ea_imm16();
    uint16_t ea_direct();
    uint16_t ea_extended();
    uint16_t ea_indexed();
    int8_t   ea_rel8();
    int16_t  ea_rel16();

    void do_bra8(int8_t off);
    void do_bra16(int16_t off);

    void mem_neg(uint16_t ea);
    void mem_com(uint16_t ea);
    void mem_lsr(uint16_t ea);
    void mem_ror(uint16_t ea);
    void mem_asr(uint16_t ea);
    void mem_lsl(uint16_t ea);
    void mem_rol(uint16_t ea);
    void mem_dec(uint16_t ea);
    void mem_inc(uint16_t ea);
    void mem_tst(uint16_t ea);
    void mem_jmp(uint16_t ea);
    void mem_clr(uint16_t ea);

    void reg_neg(uint8_t* r);
    void reg_com(uint8_t* r);
    void reg_lsr(uint8_t* r);
    void reg_ror(uint8_t* r);
    void reg_asr(uint8_t* r);
    void reg_lsl(uint8_t* r);
    void reg_rol(uint8_t* r);
    void reg_dec(uint8_t* r);
    void reg_inc(uint8_t* r);
    void reg_tst(uint8_t* r);
    void reg_clr(uint8_t* r);

    void op_suba(uint8_t v);
    void op_cmpa(uint8_t v);
    void op_sbca(uint8_t v);
    void op_anda(uint8_t v);
    void op_bita(uint8_t v);
    void op_lda(uint8_t v);
    void op_sta(uint16_t ea);
    void op_eora(uint8_t v);
    void op_adca(uint8_t v);
    void op_ora(uint8_t v);
    void op_adda(uint8_t v);

    void op_subb(uint8_t v);
    void op_cmpb(uint8_t v);
    void op_sbcb(uint8_t v);
    void op_andb(uint8_t v);
    void op_bitb(uint8_t v);
    void op_ldb(uint8_t v);
    void op_stb(uint16_t ea);
    void op_eorb(uint8_t v);
    void op_adcb(uint8_t v);
    void op_orb(uint8_t v);
    void op_addb(uint8_t v);

    void op_subd(uint16_t v);
    void op_addd(uint16_t v);
    void op_cmpx(uint16_t v);
    void op_ldx(uint16_t v);
    void op_stx(uint16_t ea);
    void op_ldd(uint16_t v);
    void op_std(uint16_t ea);
    void op_ldu(uint16_t v);
    void op_stu(uint16_t ea);
    void op_jsr(uint16_t ea);
    void op_cmpy(uint16_t v);
    void op_ldy(uint16_t v);
    void op_sty(uint16_t ea);
    void op_lds(uint16_t v);
    void op_sts(uint16_t ea);
    void op_cmpu(uint16_t v);
    void op_cmps(uint16_t v);

    void op_bsr();
    void op_sex();
    void op_daa();
    void op_orcc();
    void op_andcc();
    void op_exg();
    void op_tfr();
    void op_nop();
    void op_sync();
    void op_lbra();
    void op_lbsr();
    void op_rts();
    void op_rti();
    void op_swi();
    void op_swi2();
    void op_swi3();
    void op_mul();
    void op_abx();
    void op_cwai();
    void op_pshs();
    void op_puls();
    void op_pshu();
    void op_pulu();
    void op_leax();
    void op_leay();
    void op_leas();
    void op_leau();

    void dispatch_mem_ref();
    void dispatch_inherent_a();
    void dispatch_inherent_b();
    void dispatch_30_3f();
    void dispatch_branch();
    void dispatch_alu_a();
    void dispatch_alu_b();

    void page_2();
    void page_3();

    static void (CPU6809::*opcode_table[MAX_OPCODE])();
};

#endif
