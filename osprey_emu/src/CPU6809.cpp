#include "CPU6809.h"
#include <iostream>

//#define CPU6809_DEBUG
#ifdef CPU6809_DEBUG
#include <cstdio>
#include <cstring>
static const char* mnemonic_table[256] = {
    "NEG",    nullptr,  nullptr,  "COM",    "LSR",    nullptr,  "ROR",    "ASR",
    "LSL",    "ROL",    "DEC",    nullptr,  "INC",    "TST",    "JMP",    "CLR",
    "PAGE2",  "PAGE3",  "NOP",    "SYNC",   nullptr,  nullptr,  "LBRA",   "LBSR",
    nullptr,  "DAA",    "ORCC",   nullptr,  "ANDCC",  "SEX",    "EXG",    "TFR",
    "BRA",    "BRN",    "BHI",    "BLS",    "BCC",    "BCS",    "BNE",    "BEQ",
    "BVC",    "BVS",    "BPL",    "BMI",    "BGE",    "BLT",    "BGT",    "BLE",
    "LEAX",   "LEAY",   "LEAS",   "LEAU",   "PSHS",   "PULS",   "PSHU",   "PULU",
    nullptr,  "RTS",    "ABX",    "RTI",    "CWAI",   "MUL",    nullptr,  "SWI",
    "NEGA",   nullptr,  nullptr,  "COMA",   "LSRA",   nullptr,  "RORA",   "ASRA",
    "LSLA",   "ROLA",   "DECA",   nullptr,  "INCA",   "TSTA",   nullptr,  "CLRA",
    "NEGB",   nullptr,  nullptr,  "COMB",   "LSRB",   nullptr,  "RORB",   "ASRB",
    "LSLB",   "ROLB",   "DECB",   nullptr,  "INCB",   "TSTB",   nullptr,  "CLRB",
    "NEG",    nullptr,  nullptr,  "COM",    "LSR",    nullptr,  "ROR",    "ASR",
    "LSL",    "ROL",    "DEC",    nullptr,  "INC",    "TST",    "JMP",    "CLR",
    "NEG",    nullptr,  nullptr,  "COM",    "LSR",    nullptr,  "ROR",    "ASR",
    "LSL",    "ROL",    "DEC",    nullptr,  "INC",    "TST",    "JMP",    "CLR",
    "SUBA",   "CMPA",   "SBCA",   "SUBD",   "ANDA",   "BITA",   "LDA",    nullptr,
    "EORA",   "ADCA",   "ORA",    "ADDA",   "CMPX",   "BSR",    "LDX",    nullptr,
    "SUBA",   "CMPA",   "SBCA",   "SUBD",   "ANDA",   "BITA",   "LDA",    "STA",
    "EORA",   "ADCA",   "ORA",    "ADDA",   "CMPX",   "JSR",    "LDX",    "STX",
    "SUBA",   "CMPA",   "SBCA",   "SUBD",   "ANDA",   "BITA",   "LDA",    "STA",
    "EORA",   "ADCA",   "ORA",    "ADDA",   "CMPX",   "JSR",    "LDX",    "STX",
    "SUBA",   "CMPA",   "SBCA",   "SUBD",   "ANDA",   "BITA",   "LDA",    "STA",
    "EORA",   "ADCA",   "ORA",    "ADDA",   "CMPX",   "JSR",    "LDX",    "STX",
    "SUBB",   "CMPB",   "SBCB",   "ADDD",   "ANDB",   "BITB",   "LDB",    nullptr,
    "EORB",   "ADCB",   "ORB",    "ADDB",   "LDD",    nullptr,  "LDU",    nullptr,
    "SUBB",   "CMPB",   "SBCB",   "ADDD",   "ANDB",   "BITB",   "LDB",    "STB",
    "EORB",   "ADCB",   "ORB",    "ADDB",   "LDD",    "STD",    "LDU",    "STU",
    "SUBB",   "CMPB",   "SBCB",   "ADDD",   "ANDB",   "BITB",   "LDB",    "STB",
    "EORB",   "ADCB",   "ORB",    "ADDB",   "LDD",    "STD",    "LDU",    "STU",
    "SUBB",   "CMPB",   "SBCB",   "ADDD",   "ANDB",   "BITB",   "LDB",    "STB",
    "EORB",   "ADCB",   "ORB",    "ADDB",   "LDD",    "STD",    "LDU",    "STU",
};

#endif

CPU6809::CPU6809(uint8_t(*rmf)(uint16_t addr), void(*wmf)(uint16_t addr, uint8_t byte))
    : read_mem(rmf), write_mem(wmf), cc(0), dp(0), zero(0), reset(true), m_irq(false), ir(0)
{
    regs_8[reg_a] = reinterpret_cast<uint8_t*>(&regs_16[reg_d]) + 1;
    regs_8[reg_b] = reinterpret_cast<uint8_t*>(&regs_16[reg_d]);
    regs_8[reg_cc] = &cc;
    regs_8[reg_dp] = &dp;
    regs_8[reg_0_1] = &zero;
    regs_8[reg_0_2] = &zero;
    regs_8[reg_e] = reinterpret_cast<uint8_t*>(&regs_16[reg_w]) + 1;
    regs_8[reg_f] = reinterpret_cast<uint8_t*>(&regs_16[reg_w]);
	reset = true;
}

CPU6809::~CPU6809() {}

void CPU6809::assert_irq() { m_irq = true; }

uint16_t CPU6809::read16(uint16_t addr) {
    return ((uint16_t)read_mem(addr) << 8) | read_mem(addr + 1);
}

void CPU6809::write16(uint16_t addr, uint16_t val) {
    write_mem(addr, val >> 8);
    write_mem(addr + 1, val & 0xff);
}

void CPU6809::set_nz8(uint8_t v) {
    cc &= ~(CC_N | CC_Z);
    if (v & 0x80) cc |= CC_N;
    if (v == 0) cc |= CC_Z;
}

void CPU6809::set_nz16(uint16_t v) {
    cc &= ~(CC_N | CC_Z);
    if (v & 0x8000) cc |= CC_N;
    if (v == 0) cc |= CC_Z;
}

void CPU6809::spush(uint8_t v) { regs_16[reg_s]--; write_mem(regs_16[reg_s], v); }
uint8_t CPU6809::spop() { uint8_t v = read_mem(regs_16[reg_s]); regs_16[reg_s]++; return v; }
void CPU6809::upush(uint8_t v) { regs_16[reg_u]--; write_mem(regs_16[reg_u], v); }
uint8_t CPU6809::upop() { uint8_t v = read_mem(regs_16[reg_u]); regs_16[reg_u]++; return v; }
void CPU6809::spush16(uint16_t v) {spush(v & 0xff); spush(v >> 8);  }
uint16_t CPU6809::spop16() { uint8_t hi = spop(); return ((uint16_t)hi << 8) | spop(); }
void CPU6809::upush16(uint16_t v) { upush(v & 0xff); upush(v >> 8);}
uint16_t CPU6809::upop16() { uint8_t hi = upop(); return ((uint16_t)hi << 8) | upop(); }

uint8_t CPU6809::ea_imm8() { return read_mem(regs_16[reg_pc]++); }
uint16_t CPU6809::ea_imm16() { uint16_t v = read16(regs_16[reg_pc]); regs_16[reg_pc] += 2; return v; }
uint16_t CPU6809::ea_direct() { uint16_t a = read_mem(regs_16[reg_pc]++); a |= ((uint16_t)dp) << 8; return a; }

uint16_t CPU6809::ea_extended() {
    uint16_t a = read16(regs_16[reg_pc]);
    regs_16[reg_pc] += 2;
    return a;
}

uint16_t CPU6809::ea_indexed() {
	
    uint8_t post = read_mem(regs_16[reg_pc]++);
    

    
    uint8_t reg_sel = (post >> 5) & 0b11;
    bool indirect = (post & 0x10) != 0;
    uint8_t mode = post & 0x0F;

    uint16_t* baseregs[] = { &regs_16[reg_x], &regs_16[reg_y], &regs_16[reg_u], &regs_16[reg_s] };
    uint16_t* base = baseregs[reg_sel];

    uint16_t ea = 0;
    
    if(!(post & 0x80)){
		//5 bit off
		
		uint16_t signed_off = (post & 0x1f);
		if(signed_off & 0x10) signed_off |= 0xffe0;

		ea = *base + signed_off;
	}
	else{

		switch (mode) {
			case 0x00: ea = (*base)++; break;
			case 0x01: ea = *base; *base += 2; break;
			case 0x02: ea = --(*base); break;
			case 0x03: *base -= 2; ea = *base; break;
			case 0x04: ea = *base; break;
			case 0x05: ea = *base + *regs_8[reg_b]; break;
			case 0x06: ea = *base + *regs_8[reg_a]; break;
			case 0x08: ea = *base + (int8_t)read_mem(regs_16[reg_pc]++); break;
			case 0x09: ea = *base + read16(regs_16[reg_pc]); regs_16[reg_pc] += 2; break;
			case 0x0B: ea = *base + regs_16[reg_d]; break;
			case 0x0C: { int8_t off = (int8_t)read_mem(regs_16[reg_pc]++); ea = regs_16[reg_pc] + off; break; }
			case 0x0D: { int16_t off = (int16_t)read16(regs_16[reg_pc]); regs_16[reg_pc] += 2; ea = regs_16[reg_pc] + off; break; }
			case 0x0F: ea = read16(regs_16[reg_pc]); regs_16[reg_pc] += 2; break;
			default: ea = *base; break;
		}

		if (indirect) ea = read16(ea);
	}
    return ea;
}

int8_t CPU6809::ea_rel8() { return (int8_t)read_mem(regs_16[reg_pc]++); }
int16_t CPU6809::ea_rel16() { int16_t v = (int16_t)read16(regs_16[reg_pc]); regs_16[reg_pc] += 2; return v; }
void CPU6809::do_bra8(int8_t off) { regs_16[reg_pc] += off; }
void CPU6809::do_bra16(int16_t off) { regs_16[reg_pc] += off; }

void CPU6809::mem_neg(uint16_t ea) { uint8_t v = read_mem(ea); cc |= CC_C; cc &= ~CC_V; if (v == 0x80) cc |= CC_V; if (v == 0) cc &= ~CC_C; v = (uint8_t)(-(int8_t)v); write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_com(uint16_t ea) { uint8_t v = ~read_mem(ea); write_mem(ea, v); cc &= ~CC_V; cc |= CC_C; set_nz8(v); }
void CPU6809::mem_lsr(uint16_t ea) { uint8_t v = read_mem(ea); cc = (cc & ~CC_C) | (v & 1); v >>= 1; write_mem(ea, v); cc &= ~CC_N; if (v == 0) cc |= CC_Z; }
void CPU6809::mem_ror(uint16_t ea) { uint8_t v = read_mem(ea); uint8_t oc = cc & CC_C; cc = (cc & ~CC_C) | (v & 1); v >>= 1; if (oc) v |= 0x80; write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_asr(uint16_t ea) { uint8_t v = read_mem(ea); cc = (cc & ~CC_C) | (v & 1); v = (v >> 1) | (v & 0x80); write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_lsl(uint16_t ea) { uint8_t v = read_mem(ea); cc = (cc & ~CC_C) | ((v & 0x80) ? CC_C : 0); cc &= ~CC_V; if (!!(v & 0x40) ^ !!(v & 0x80)) cc |= CC_V; v <<= 1; write_mem(ea, v); cc &= ~CC_N; if (v == 0) cc |= CC_Z; }
void CPU6809::mem_rol(uint16_t ea) { uint8_t v = read_mem(ea); uint8_t oc = cc & CC_C; cc = (cc & ~CC_C) | ((v & 0x80) ? CC_C : 0); v <<= 1; if (oc) v |= 1; cc &= ~CC_V; if (!!(v & 0x40) ^ !!(v & 0x80)) cc |= CC_V; write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_dec(uint16_t ea) { uint8_t v = read_mem(ea); cc &= ~CC_V; if (v == 0x80) cc |= CC_V; v--; write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_inc(uint16_t ea) { uint8_t v = read_mem(ea); cc &= ~CC_V; if (v == 0x7f) cc |= CC_V; v++; write_mem(ea, v); set_nz8(v); }
void CPU6809::mem_tst(uint16_t ea) { uint8_t v = read_mem(ea); cc &= ~CC_V; set_nz8(v); }
void CPU6809::mem_jmp(uint16_t ea) { regs_16[reg_pc] = ea; }
void CPU6809::mem_clr(uint16_t ea) { write_mem(ea, 0); cc &= ~(CC_N | CC_V | CC_C); cc |= CC_Z; }

void CPU6809::reg_neg(uint8_t* r) { uint8_t v = *r; cc |= CC_C; cc &= ~CC_V; if (v == 0x80) cc |= CC_V; if (v == 0) cc &= ~CC_C; *r = (uint8_t)(-(int8_t)v); set_nz8(*r); }
void CPU6809::reg_com(uint8_t* r) { *r = ~(*r); cc &= ~CC_V; cc |= CC_C; set_nz8(*r); }
void CPU6809::reg_lsr(uint8_t* r) { uint8_t v = *r; cc = (cc & ~CC_C) | (v & 1); v >>= 1; *r = v; cc &= ~CC_N; if (v == 0) cc |= CC_Z; }
void CPU6809::reg_ror(uint8_t* r) { uint8_t v = *r; uint8_t oc = cc & CC_C; cc = (cc & ~CC_C) | (v & 1); v >>= 1; if (oc) v |= 0x80; *r = v; set_nz8(v); }
void CPU6809::reg_asr(uint8_t* r) { uint8_t v = *r; cc = (cc & ~CC_C) | (v & 1); v = (v >> 1) | (v & 0x80); *r = v; set_nz8(v); }
void CPU6809::reg_lsl(uint8_t* r) { uint8_t v = *r; cc = (cc & ~CC_C) | ((v & 0x80) ? CC_C : 0); cc &= ~CC_V; if (!!(v & 0x40) ^ !!(v & 0x80)) cc |= CC_V; v <<= 1; *r = v; cc &= ~CC_N; if (v == 0) cc |= CC_Z; }
void CPU6809::reg_rol(uint8_t* r) { uint8_t v = *r; uint8_t oc = cc & CC_C; cc = (cc & ~CC_C) | ((v & 0x80) ? CC_C : 0); v <<= 1; if (oc) v |= 1; *r = v; cc &= ~CC_V; if (!!(v & 0x40) ^ !!(v & 0x80)) cc |= CC_V; set_nz8(v); }
void CPU6809::reg_dec(uint8_t* r) { uint8_t v = *r; cc &= ~CC_V; if (v == 0x80) cc |= CC_V; (*r)--; set_nz8(*r); }
void CPU6809::reg_inc(uint8_t* r) { uint8_t v = *r; cc &= ~CC_V; if (v == 0x7f) cc |= CC_V; (*r)++; set_nz8(*r); }
void CPU6809::reg_tst(uint8_t* r) { cc &= ~CC_V; set_nz8(*r); }
void CPU6809::reg_clr(uint8_t* r) { *r = 0; cc &= ~(CC_N | CC_V | CC_C); cc |= CC_Z; }

void CPU6809::op_suba(uint8_t v) {
    uint8_t a = *regs_8[reg_a];
    uint16_t r = (uint16_t)a - (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((a ^ v) & (a ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(a & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(a & 0x8)) cc |= CC_H;
    *regs_8[reg_a] = (uint8_t)r;
    set_nz8(*regs_8[reg_a]);
}

void CPU6809::op_cmpa(uint8_t v) {
    uint8_t a = *regs_8[reg_a];
    uint16_t r = (uint16_t)a - (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((a ^ v) & (a ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(a & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(a & 0x8)) cc |= CC_H;
    if ((uint8_t)r == 0) cc |= CC_Z;
    if ((uint8_t)r & 0x80) cc |= CC_N;
}

void CPU6809::op_sbca(uint8_t v) {
    uint8_t a = *regs_8[reg_a];
    uint8_t cy = (cc & CC_C) ? 1 : 0;
    uint16_t r = (uint16_t)a - (uint16_t)v - cy;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((a ^ v) & (a ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(a & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(a & 0x8)) cc |= CC_H;
    *regs_8[reg_a] = (uint8_t)r;
    set_nz8(*regs_8[reg_a]);
}

void CPU6809::op_anda(uint8_t v) { *regs_8[reg_a] &= v; cc &= ~CC_V; set_nz8(*regs_8[reg_a]); }
void CPU6809::op_bita(uint8_t v) { uint8_t r = *regs_8[reg_a] & v; cc &= ~CC_V; cc = (cc & ~(CC_Z | CC_N)); if (r == 0) cc |= CC_Z; if (r & 0x80) cc |= CC_N; }
void CPU6809::op_lda(uint8_t v) { *regs_8[reg_a] = v; cc &= ~CC_V; set_nz8(*regs_8[reg_a]); }
void CPU6809::op_sta(uint16_t ea) { write_mem(ea, *regs_8[reg_a]); cc &= ~CC_V; set_nz8(*regs_8[reg_a]); }
void CPU6809::op_eora(uint8_t v) { *regs_8[reg_a] ^= v; cc &= ~CC_V; set_nz8(*regs_8[reg_a]); }

void CPU6809::op_adca(uint8_t v) {
    uint8_t a = *regs_8[reg_a];
    uint8_t cy = (cc & CC_C) ? 1 : 0;
    uint16_t r = (uint16_t)a + (uint16_t)v + cy;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r > 0xff) cc |= CC_C;
    if (((a ^ r) & (v ^ r)) & 0x80) cc |= CC_V;
    if (((a & 0xf) + (v & 0xf) + cy) > 0xf) cc |= CC_H;
    *regs_8[reg_a] = (uint8_t)r;
    set_nz8(*regs_8[reg_a]);
}

void CPU6809::op_ora(uint8_t v) { *regs_8[reg_a] |= v; cc &= ~CC_V; set_nz8(*regs_8[reg_a]); }

void CPU6809::op_adda(uint8_t v) {
    uint8_t a = *regs_8[reg_a];
    uint16_t r = (uint16_t)a + (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r > 0xff) cc |= CC_C;
    if (((a ^ r) & (v ^ r)) & 0x80) cc |= CC_V;
    if (((a & 0xf) + (v & 0xf)) > 0xf) cc |= CC_H;
    *regs_8[reg_a] = (uint8_t)r;
    set_nz8(*regs_8[reg_a]);
}

void CPU6809::op_subb(uint8_t v) {
    uint8_t b = *regs_8[reg_b];
    uint16_t r = (uint16_t)b - (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((b ^ v) & (b ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(b & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(b & 0x8)) cc |= CC_H;
    *regs_8[reg_b] = (uint8_t)r;
    set_nz8(*regs_8[reg_b]);
}

void CPU6809::op_cmpb(uint8_t v) {
    uint8_t b = *regs_8[reg_b];
    uint16_t r = (uint16_t)b - (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((b ^ v) & (b ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(b & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(b & 0x8)) cc |= CC_H;
    if ((uint8_t)r == 0) cc |= CC_Z;
    if ((uint8_t)r & 0x80) cc |= CC_N;
}

void CPU6809::op_sbcb(uint8_t v) {
    uint8_t b = *regs_8[reg_b];
    uint8_t cy = (cc & CC_C) ? 1 : 0;
    uint16_t r = (uint16_t)b - (uint16_t)v - cy;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r & 0xff00) cc |= CC_C;
    if (((b ^ v) & (b ^ (uint8_t)r)) & 0x80) cc |= CC_V;
    if (!(b & 0x8) && (v & 0x8)) cc |= CC_H; else if ((v & 0x8) && ((uint8_t)r & 0x8)) cc |= CC_H; else if (((uint8_t)r & 0x8) && !(b & 0x8)) cc |= CC_H;
    *regs_8[reg_b] = (uint8_t)r;
    set_nz8(*regs_8[reg_b]);
}

void CPU6809::op_andb(uint8_t v) { *regs_8[reg_b] &= v; cc &= ~CC_V; set_nz8(*regs_8[reg_b]); }
void CPU6809::op_bitb(uint8_t v) { uint8_t r = *regs_8[reg_b] & v; cc &= ~CC_V; cc = (cc & ~(CC_Z | CC_N)); if (r == 0) cc |= CC_Z; if (r & 0x80) cc |= CC_N; }
void CPU6809::op_ldb(uint8_t v) { *regs_8[reg_b] = v; cc &= ~CC_V; set_nz8(*regs_8[reg_b]); }
void CPU6809::op_stb(uint16_t ea) { write_mem(ea, *regs_8[reg_b]); cc &= ~CC_V; set_nz8(*regs_8[reg_b]); }

void CPU6809::op_eorb(uint8_t v) { *regs_8[reg_b] ^= v; cc &= ~CC_V; set_nz8(*regs_8[reg_b]); }

void CPU6809::op_adcb(uint8_t v) {
    uint8_t b = *regs_8[reg_b];
    uint8_t cy = (cc & CC_C) ? 1 : 0;
    uint16_t r = (uint16_t)b + (uint16_t)v + cy;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r > 0xff) cc |= CC_C;
    if (((b ^ r) & (v ^ r)) & 0x80) cc |= CC_V;
    if (((b & 0xf) + (v & 0xf) + cy) > 0xf) cc |= CC_H;
    *regs_8[reg_b] = (uint8_t)r;
    set_nz8(*regs_8[reg_b]);
}

void CPU6809::op_orb(uint8_t v) { *regs_8[reg_b] |= v; cc &= ~CC_V; set_nz8(*regs_8[reg_b]); }

void CPU6809::op_addb(uint8_t v) {
    uint8_t b = *regs_8[reg_b];
    uint16_t r = (uint16_t)b + (uint16_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N | CC_H));
    if (r > 0xff) cc |= CC_C;
    if (((b ^ r) & (v ^ r)) & 0x80) cc |= CC_V;
    if (((b & 0xf) + (v & 0xf)) > 0xf) cc |= CC_H;
    *regs_8[reg_b] = (uint8_t)r;
    set_nz8(*regs_8[reg_b]);
}

void CPU6809::op_subd(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_d] - (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r & 0xffff0000) cc |= CC_C;
    if (((regs_16[reg_d] ^ v) & (regs_16[reg_d] ^ (uint16_t)r)) & 0x8000) cc |= CC_V;
    regs_16[reg_d] = (uint16_t)r;
    set_nz16(regs_16[reg_d]);
}

void CPU6809::op_addd(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_d] + (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r > 0xffff) cc |= CC_C;
    if (((regs_16[reg_d] ^ r) & (v ^ r)) & 0x8000) cc |= CC_V;
    regs_16[reg_d] = (uint16_t)r;
    set_nz16(regs_16[reg_d]);
}

void CPU6809::op_cmpx(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_x] - (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r & 0xffff0000) cc |= CC_C;
    if (((regs_16[reg_x] ^ v) & (regs_16[reg_x] ^ (uint16_t)r)) & 0x8000) cc |= CC_V;
    if ((uint16_t)r == 0) cc |= CC_Z;
    if ((uint16_t)r & 0x8000) cc |= CC_N;
}

void CPU6809::op_ldx(uint16_t v) { regs_16[reg_x] = v; cc &= ~CC_V; set_nz16(v); }
void CPU6809::op_stx(uint16_t ea) { write16(ea, regs_16[reg_x]); cc &= ~CC_V; set_nz16(regs_16[reg_x]); }
void CPU6809::op_ldd(uint16_t v) { regs_16[reg_d] = v; cc &= ~CC_V; set_nz16(v); }
void CPU6809::op_std(uint16_t ea) { write16(ea, regs_16[reg_d]); cc &= ~CC_V; set_nz16(regs_16[reg_d]); }
void CPU6809::op_ldu(uint16_t v) { regs_16[reg_u] = v; cc &= ~CC_V; set_nz16(v); }
void CPU6809::op_stu(uint16_t ea) { write16(ea, regs_16[reg_u]); cc &= ~CC_V; set_nz16(regs_16[reg_u]); }
void CPU6809::op_jsr(uint16_t ea) { spush16(regs_16[reg_pc]); regs_16[reg_pc] = ea; }

void CPU6809::op_cmpy(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_y] - (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r & 0xffff0000) cc |= CC_C;
    if (((regs_16[reg_y] ^ v) & (regs_16[reg_y] ^ (uint16_t)r)) & 0x8000) cc |= CC_V;
    if ((uint16_t)r == 0) cc |= CC_Z;
    if ((uint16_t)r & 0x8000) cc |= CC_N;
}
void CPU6809::op_ldy(uint16_t v) { regs_16[reg_y] = v; cc &= ~CC_V; set_nz16(v); }
void CPU6809::op_sty(uint16_t ea) { write16(ea, regs_16[reg_y]); cc &= ~CC_V; set_nz16(regs_16[reg_y]); }
void CPU6809::op_lds(uint16_t v) { regs_16[reg_s] = v; cc &= ~CC_V; set_nz16(v); }
void CPU6809::op_sts(uint16_t ea) { write16(ea, regs_16[reg_s]); cc &= ~CC_V; set_nz16(regs_16[reg_s]); }

void CPU6809::op_cmpu(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_u] - (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r & 0xffff0000) cc |= CC_C;
    if (((regs_16[reg_u] ^ v) & (regs_16[reg_u] ^ (uint16_t)r)) & 0x8000) cc |= CC_V;
    if ((uint16_t)r == 0) cc |= CC_Z;
    if ((uint16_t)r & 0x8000) cc |= CC_N;
}
void CPU6809::op_cmps(uint16_t v) {
    uint32_t r = (uint32_t)regs_16[reg_s] - (uint32_t)v;
    cc = (cc & ~(CC_C | CC_V | CC_Z | CC_N));
    if (r & 0xffff0000) cc |= CC_C;
    if (((regs_16[reg_s] ^ v) & (regs_16[reg_s] ^ (uint16_t)r)) & 0x8000) cc |= CC_V;
    if ((uint16_t)r == 0) cc |= CC_Z;
    if ((uint16_t)r & 0x8000) cc |= CC_N;
}

void CPU6809::op_bsr() { int8_t off = ea_rel8(); spush16(regs_16[reg_pc]); regs_16[reg_pc] += off; }

void CPU6809::op_sex() {
    set_nz8(*regs_8[reg_b]);
    *regs_8[reg_a] = (*regs_8[reg_b] & 0x80) ? 0xff : 0;
}

void CPU6809::op_daa() {
    uint8_t a = *regs_8[reg_a];
    uint8_t hi = a >> 4, lo = a & 0x0f;
    uint8_t old_hi = hi;
    if ((cc & CC_C) || hi > 9 || (hi > 8 && lo > 9)) hi += 6;
    if ((cc & CC_H) || lo > 9) lo += 6;
    cc &= ~CC_C;
    if (old_hi > 9) cc |= CC_C;
    *regs_8[reg_a] = ((hi & 0xf) << 4) | (lo & 0xf);
    set_nz8(*regs_8[reg_a]);
}

void CPU6809::op_orcc() { cc |= read_mem(regs_16[reg_pc]++); }
void CPU6809::op_andcc() { cc &= read_mem(regs_16[reg_pc]++); }

void CPU6809::op_exg() {
    uint8_t imm = read_mem(regs_16[reg_pc]++);
    uint8_t r0 = imm >> 4, r1 = imm & 0x0f;
    if ((r0 & 8) && (r1 & 8)) { uint8_t t = *regs_8[r0 & 7]; *regs_8[r0 & 7] = *regs_8[r1 & 7]; *regs_8[r1 & 7] = t; }
    else if (!(r0 & 8) && !(r1 & 8)) { uint16_t t = regs_16[r0]; regs_16[r0] = regs_16[r1]; regs_16[r1] = t; }
    else if ((r0 & 8) && !(r1 & 8)) { uint8_t t = *regs_8[r0 & 7]; *regs_8[r0 & 7] = regs_16[r1] & 0xff; regs_16[r1] = 0xff00 | t; }
    else { uint8_t t = *regs_8[r1 & 7]; *regs_8[r1 & 7] = regs_16[r0] & 0xff; regs_16[r0] = 0xff00 | t; }
}

void CPU6809::op_tfr() {
    uint8_t imm = read_mem(regs_16[reg_pc]++);
    uint8_t r0 = imm >> 4, r1 = imm & 0x0f;
    if ((r0 & 8) && (r1 & 8)) { *regs_8[r1 & 7] = *regs_8[r0 & 7]; }
    else if (!(r0 & 8) && !(r1 & 8)) { regs_16[r1] = regs_16[r0]; }
    else if ((r0 & 8) && !(r1 & 8)) { regs_16[r1] = 0xff00 | *regs_8[r0 & 7]; }
    else { *regs_8[r1 & 7] = regs_16[r0] & 0xff; }
}

void CPU6809::op_nop() {}
void CPU6809::op_sync() {}

void CPU6809::op_lbra()  { int16_t off = ea_rel16(); regs_16[reg_pc] += off; }
void CPU6809::op_lbsr()  { int16_t off = ea_rel16(); spush16(regs_16[reg_pc]); regs_16[reg_pc] += off; }

void CPU6809::op_rts()   { regs_16[reg_pc] = spop16(); }

void CPU6809::op_rti() {
    cc = spop();
    if (cc & CC_E) {
        dp = spop();
        *regs_8[reg_b] = spop();
        *regs_8[reg_a] = spop();
        regs_16[reg_x] = spop16();
        regs_16[reg_y] = spop16();
        regs_16[reg_u] = spop16();
    }
    regs_16[reg_pc] = spop16();
}

void CPU6809::op_swi() {
    cc |= CC_E;
    spush16(regs_16[reg_pc]);
    spush16(regs_16[reg_u]);
    spush16(regs_16[reg_y]);
    spush16(regs_16[reg_x]);
    spush(regs_16[reg_d] >> 8);
    spush(regs_16[reg_d] & 0xff);
    spush(dp);
    spush(cc);
    cc |= CC_I | CC_F;
    regs_16[reg_pc] = read16(0xfffa);
}

void CPU6809::op_swi2() {
    cc |= CC_E;
    spush16(regs_16[reg_pc]);
    spush16(regs_16[reg_u]);
    spush16(regs_16[reg_y]);
    spush16(regs_16[reg_x]);
    spush(regs_16[reg_d] >> 8);
    spush(regs_16[reg_d] & 0xff);
    spush(dp);
    spush(cc);
    regs_16[reg_pc] = read16(0xfff4);
}

void CPU6809::op_swi3() {
    cc |= CC_E;
    spush16(regs_16[reg_pc]);
    spush16(regs_16[reg_u]);
    spush16(regs_16[reg_y]);
    spush16(regs_16[reg_x]);
    spush(regs_16[reg_d] >> 8);
    spush(regs_16[reg_d] & 0xff);
    spush(dp);
    spush(cc);
    regs_16[reg_pc] = read16(0xfff2);
}

void CPU6809::op_mul() {
    uint16_t r = (uint16_t)(*regs_8[reg_a]) * (uint16_t)(*regs_8[reg_b]);
    regs_16[reg_d] = r;
    cc &= ~CC_C;
    if (r & 0x80) cc |= CC_C;
    set_nz16(r);
}

void CPU6809::op_abx() { regs_16[reg_x] += *regs_8[reg_b]; }

void CPU6809::op_cwai() {
    uint8_t imm = read_mem(regs_16[reg_pc]++);
    cc &= imm;
    cc |= CC_E;
    spush16(regs_16[reg_pc]);
    spush16(regs_16[reg_u]);
    spush16(regs_16[reg_y]);
    spush16(regs_16[reg_x]);
    spush(regs_16[reg_d] >> 8);
    spush(regs_16[reg_d] & 0xff);
    spush(dp);
    spush(cc);
}

void CPU6809::op_pshs() {
    uint8_t m = read_mem(regs_16[reg_pc]++);
    if (m & 0x80) spush16(regs_16[reg_pc]);
    if (m & 0x40) spush16(regs_16[reg_u]);
    if (m & 0x20) spush16(regs_16[reg_y]);
    if (m & 0x10) spush16(regs_16[reg_x]);
    if (m & 0x08) spush(dp);
    if (m & 0x04) spush(*regs_8[reg_b]);
    if (m & 0x02) spush(*regs_8[reg_a]);
    if (m & 0x01) spush(cc);
}

void CPU6809::op_puls() {
    uint8_t m = read_mem(regs_16[reg_pc]++);
    if (m & 0x01) cc = spop();
    if (m & 0x02) *regs_8[reg_a] = spop();
    if (m & 0x04) *regs_8[reg_b] = spop();
    if (m & 0x08) dp = spop();
    if (m & 0x10) regs_16[reg_x] = spop16();
    if (m & 0x20) regs_16[reg_y] = spop16();
    if (m & 0x40) regs_16[reg_u] = spop16();
    if (m & 0x80) regs_16[reg_pc] = spop16();
}

void CPU6809::op_pshu() {
    uint8_t m = read_mem(regs_16[reg_pc]++);
    if (m & 0x80) upush16(regs_16[reg_pc]);
    if (m & 0x40) upush16(regs_16[reg_s]);
    if (m & 0x20) upush16(regs_16[reg_y]);
    if (m & 0x10) upush16(regs_16[reg_x]);
    if (m & 0x08) upush(dp);
    if (m & 0x04) spush(*regs_8[reg_b]);
    if (m & 0x02) spush(*regs_8[reg_a]);
    if (m & 0x01) upush(cc);
}

void CPU6809::op_pulu() {
    uint8_t m = read_mem(regs_16[reg_pc]++);
    if (m & 0x01) cc = upop();
    if (m & 0x02) *regs_8[reg_a] = upop();
    if (m & 0x04) *regs_8[reg_b] = upop();
    if (m & 0x08) dp = upop();
    if (m & 0x10) regs_16[reg_x] = upop16();
    if (m & 0x20) regs_16[reg_y] = upop16();
    if (m & 0x40) regs_16[reg_s] = upop16();
    if (m & 0x80) regs_16[reg_pc] = upop16();
}

void CPU6809::op_leax() {
    uint8_t pb = read_mem(regs_16[reg_pc]);
    if (pb == 0x84) {
        
		//very crude breakpoint
		bool quit = false;
		
		while(!quit){
			
			uint32_t addr;
			
			std::cin >>std::hex>> addr;
			if(addr == 0xffffffff){
				quit = true;
			}
			
			std::cout << std::hex <<(uint16_t)read_mem(addr)<<std::endl;
		}

    }
    regs_16[reg_x] = ea_indexed(); cc &= ~CC_Z; if (regs_16[reg_x] == 0) cc |= CC_Z;
}
void CPU6809::op_leay() { regs_16[reg_y] = ea_indexed(); cc &= ~CC_Z; if (regs_16[reg_y] == 0) cc |= CC_Z; }
void CPU6809::op_leas() { regs_16[reg_s] = ea_indexed(); }
void CPU6809::op_leau() { regs_16[reg_u] = ea_indexed(); }

void CPU6809::dispatch_mem_ref() {
    uint8_t hi = (ir >> 4) & 0x0f;
    uint8_t lo = ir & 0x0f;
    uint16_t ea = 0;
    switch (hi) { case 0x0: ea = ea_direct(); break; case 0x6: ea = ea_indexed(); break; case 0x7: ea = ea_extended(); break; default: return; }
    switch (lo) {
        case 0x0: mem_neg(ea); break; case 0x3: mem_com(ea); break; case 0x4: mem_lsr(ea); break;
        case 0x6: mem_ror(ea); break; case 0x7: mem_asr(ea); break; case 0x8: mem_lsl(ea); break;
        case 0x9: mem_rol(ea); break; case 0xA: mem_dec(ea); break; case 0xC: mem_inc(ea); break;
        case 0xD: mem_tst(ea); break; case 0xE: mem_jmp(ea); break; case 0xF: mem_clr(ea); break;
        default: break;
    }
}

void CPU6809::dispatch_inherent_a() {
    uint8_t lo = ir & 0x0f;
    uint8_t* r = regs_8[reg_a];
    switch (lo) {
        case 0x0: reg_neg(r); break; case 0x3: reg_com(r); break; case 0x4: reg_lsr(r); break;
        case 0x6: reg_ror(r); break; case 0x7: reg_asr(r); break; case 0x8: reg_lsl(r); break;
        case 0x9: reg_rol(r); break; case 0xA: reg_dec(r); break; case 0xC: reg_inc(r); break;
        case 0xD: reg_tst(r); break; case 0xF: reg_clr(r); break;
        default: break;
    }
}

void CPU6809::dispatch_inherent_b() {
    uint8_t lo = ir & 0x0f;
    uint8_t* r = regs_8[reg_b];
    switch (lo) {
        case 0x0: reg_neg(r); break; case 0x3: reg_com(r); break; case 0x4: reg_lsr(r); break;
        case 0x6: reg_ror(r); break; case 0x7: reg_asr(r); break; case 0x8: reg_lsl(r); break;
        case 0x9: reg_rol(r); break; case 0xA: reg_dec(r); break; case 0xC: reg_inc(r); break;
        case 0xD: reg_tst(r); break; case 0xF: reg_clr(r); break;
        default: break;
    }
}

void CPU6809::dispatch_30_3f() {
    switch (ir) {
        case 0x30: op_leax(); break; case 0x31: op_leay(); break; case 0x32: op_leas(); break; case 0x33: op_leau(); break;
        case 0x34: op_pshs(); break; case 0x35: op_puls(); break; case 0x36: op_pshu(); break; case 0x37: op_pulu(); break;
        case 0x39: op_rts(); break; case 0x3A: op_abx(); break; case 0x3B: op_rti(); break;
        case 0x3C: op_cwai(); break; case 0x3D: op_mul(); break; case 0x3F: op_swi(); break;
        default: break;
    }
}

void CPU6809::dispatch_branch() {
    uint8_t lo = ir & 0x0f;
    int8_t off = ea_rel8();
    switch (lo) {
        case 0x0: do_bra8(off); break;
        case 0x1: break;
        case 0x2: if (!(cc & (CC_C | CC_Z))) do_bra8(off); break;
        case 0x3: if (cc & (CC_C | CC_Z)) do_bra8(off); break;
        case 0x4: if (!(cc & CC_C)) do_bra8(off); break;
        case 0x5: if (cc & CC_C) do_bra8(off); break;
        case 0x6: if (!(cc & CC_Z)) do_bra8(off); break;
        case 0x7: if (cc & CC_Z) do_bra8(off); break;
        case 0x8: if (!(cc & CC_V)) do_bra8(off); break;
        case 0x9: if (cc & CC_V) do_bra8(off); break;
        case 0xA: if (!(cc & CC_N)) do_bra8(off); break;
        case 0xB: if (cc & CC_N) do_bra8(off); break;
        case 0xC: if (((cc & CC_N) != 0) == ((cc & CC_V) != 0)) do_bra8(off); break;
        case 0xD: if (((cc & CC_N) != 0) != ((cc & CC_V) != 0)) do_bra8(off); break;
        case 0xE: if (!(cc & CC_Z) && ((cc & CC_N) != 0) == ((cc & CC_V) != 0)) do_bra8(off); break;
        case 0xF: if ((cc & CC_Z) || ((cc & CC_N) != 0) != ((cc & CC_V) != 0)) do_bra8(off); break;
    }
}

void CPU6809::dispatch_alu_a() {
    uint8_t hi = (ir >> 4) & 0x0f;
    uint8_t lo = ir & 0x0f;
    bool imm = (hi == 0x8);

    switch (lo) {
        case 0x0: if (imm) op_suba(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_suba(read_mem(e)); } break;
        case 0x1: if (imm) op_cmpa(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_cmpa(read_mem(e)); } break;
        case 0x2: if (imm) op_sbca(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_sbca(read_mem(e)); } break;
        case 0x3: if (imm) op_subd(ea_imm16()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_subd(read16(e)); } break;
        case 0x4: if (imm) op_anda(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_anda(read_mem(e)); } break;
        case 0x5: if (imm) op_bita(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_bita(read_mem(e)); } break;
        case 0x6: if (imm) op_lda(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_lda(read_mem(e)); } break;
        case 0x7: if (!imm) { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_sta(e); } break;
        case 0x8: if (imm) op_eora(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_eora(read_mem(e)); } break;
        case 0x9: if (imm) op_adca(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_adca(read_mem(e)); } break;
        case 0xA: if (imm) op_ora(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_ora(read_mem(e)); } break;
        case 0xB: if (imm) op_adda(ea_imm8()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_adda(read_mem(e)); } break;
        case 0xC: if (imm) op_cmpx(ea_imm16()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_cmpx(read16(e)); } break;
        case 0xD: if (imm) op_bsr(); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_jsr(e); } break;
        case 0xE: if (imm) op_ldx(ea_imm16()); else { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_ldx(read16(e)); } break;
        case 0xF: if (!imm) { uint16_t e = (hi==0x9)?ea_direct():(hi==0xA)?ea_indexed():ea_extended(); op_stx(e); } break;
        default: break;
    }
}

void CPU6809::dispatch_alu_b() {
    uint8_t hi = (ir >> 4) & 0x0f;
    uint8_t lo = ir & 0x0f;
    bool imm = (hi == 0xC);

    switch (lo) {
        case 0x0: if (imm) op_subb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_subb(read_mem(e)); } break;
        case 0x1: if (imm) op_cmpb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_cmpb(read_mem(e)); } break;
        case 0x2: if (imm) op_sbcb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_sbcb(read_mem(e)); } break;
        case 0x3: if (imm) op_addd(ea_imm16()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_addd(read16(e)); } break;
        case 0x4: if (imm) op_andb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_andb(read_mem(e)); } break;
        case 0x5: if (imm) op_bitb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_bitb(read_mem(e)); } break;
        case 0x6: if (imm) op_ldb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_ldb(read_mem(e)); } break;
        case 0x7: if (!imm) { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_stb(e); } break;
        case 0x8: if (imm) op_eorb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_eorb(read_mem(e)); } break;
        case 0x9: if (imm) op_adcb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_adcb(read_mem(e)); } break;
        case 0xA: if (imm) op_orb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_orb(read_mem(e)); } break;
        case 0xB: if (imm) op_addb(ea_imm8()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_addb(read_mem(e)); } break;
        case 0xC: if (imm) op_ldd(ea_imm16()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_ldd(read16(e)); } break;
        case 0xD: if (!imm) { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_std(e); } break;
        case 0xE: if (imm) op_ldu(ea_imm16()); else { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_ldu(read16(e)); } break;
        case 0xF: if (!imm) { uint16_t e = (hi==0xD)?ea_direct():(hi==0xE)?ea_indexed():ea_extended(); op_stu(e); } break;
        default: break;
    }
}

static inline bool br_hi(uint8_t cc)   { return !(cc & (0x01|0x04)); }
static inline bool br_ls(uint8_t cc)   { return (cc & (0x01|0x04)); }
static inline bool br_cc(uint8_t cc)   { return !(cc & 0x01); }
static inline bool br_cs(uint8_t cc)   { return (cc & 0x01); }
static inline bool br_ne(uint8_t cc)   { return !(cc & 0x04); }
static inline bool br_eq(uint8_t cc)   { return (cc & 0x04); }
static inline bool br_vc(uint8_t cc)   { return !(cc & 0x02); }
static inline bool br_vs(uint8_t cc)   { return (cc & 0x02); }
static inline bool br_pl(uint8_t cc)   { return !(cc & 0x08); }
static inline bool br_mi(uint8_t cc)   { return (cc & 0x08); }
static inline bool br_ge(uint8_t cc)   { return ((cc & 0x08) != 0) == ((cc & 0x02) != 0); }
static inline bool br_lt(uint8_t cc)   { return ((cc & 0x08) != 0) != ((cc & 0x02) != 0); }
static inline bool br_gt(uint8_t cc)   { return !(cc & 0x04) && ((cc & 0x08) != 0) == ((cc & 0x02) != 0); }
static inline bool br_le(uint8_t cc)   { return (cc & 0x04) || ((cc & 0x08) != 0) != ((cc & 0x02) != 0); }

void CPU6809::page_2() {
#ifdef CPU6809_DEBUG
    uint16_t base_pc = regs_16[reg_pc];
#endif
    uint8_t op = read_mem(regs_16[reg_pc]++);

    if (op >= 0x21 && op <= 0x2F) {
        int16_t off = ea_rel16();
#ifdef CPU6809_DEBUG
        static const char* lb_names[] = { nullptr, "LBRN","LBHI","LBLS","LBCC","LBCS","LBNE","LBEQ","LBVC","LBVS","LBPL","LBMI","LBGE","LBLT","LBGT","LBLE" };
#endif
        bool taken = false;
        switch (op) {
            case 0x21: break;
            case 0x22: taken = br_hi(cc); break; case 0x23: taken = br_ls(cc); break;
            case 0x24: taken = br_cc(cc); break; case 0x25: taken = br_cs(cc); break;
            case 0x26: taken = br_ne(cc); break; case 0x27: taken = br_eq(cc); break;
            case 0x28: taken = br_vc(cc); break; case 0x29: taken = br_vs(cc); break;
            case 0x2A: taken = br_pl(cc); break; case 0x2B: taken = br_mi(cc); break;
            case 0x2C: taken = br_ge(cc); break; case 0x2D: taken = br_lt(cc); break;
            case 0x2E: taken = br_gt(cc); break;             case 0x2F: taken = br_le(cc); break;
        }
        if (taken) do_bra16(off);
#ifdef CPU6809_DEBUG
        uint16_t target = base_pc + 3 + off;
        fprintf(stderr, "              10 %02X %-6s $%04X  (%s)\n", op,
                lb_names[op & 0x0F] ? lb_names[op & 0x0F] : "???",
                target, taken ? "taken" : "not taken");
#endif
        return;
    }

    if (op == 0x3F) {
#ifdef CPU6809_DEBUG
        fprintf(stderr, "              10 3F SWI2\n");
#endif
        op_swi2(); return; }

    uint8_t hi = (op >> 4) & 0x0f;
    uint8_t lo = op & 0x0f;

    if (hi == 0x8 || hi == 0x9 || hi == 0xA || hi == 0xB) {
        bool y_imm = (hi == 0x8);
        uint16_t ea = 0;
        if (!y_imm) {
            switch (hi) { case 0x9: ea = ea_direct(); break; case 0xA: ea = ea_indexed(); break; case 0xB: ea = ea_extended(); break; }
        }
        switch (lo) {
            case 0xC: if (y_imm) op_cmpy(ea_imm16()); else op_cmpy(read16(ea)); break;
            case 0xE: if (y_imm) op_ldy(ea_imm16()); else op_ldy(read16(ea)); break;
            case 0xF: if (!y_imm) op_sty(ea); break;
            default: break;
        }
    }

    if (hi == 0xC || hi == 0xD || hi == 0xE || hi == 0xF) {
        uint16_t ea = 0;
        bool low_imm = (hi == 0xC);
        if (!low_imm) {
            switch (hi) { case 0xD: ea = ea_direct(); break; case 0xE: ea = ea_indexed(); break; case 0xF: ea = ea_extended(); break; }
        }
        switch (lo) {
            case 0xE: if (low_imm) op_lds(ea_imm16()); else op_lds(read16(ea)); break;
            case 0xF: if (!low_imm) op_sts(ea); break;
            default: break;
        }
    }
}

void CPU6809::page_3() {
    uint8_t op = read_mem(regs_16[reg_pc]++);
    if (op == 0x3F) {
#ifdef CPU6809_DEBUG
        fprintf(stderr, "              11 3F SWI3\n");
#endif
        op_swi3(); return; }

    uint8_t hi = (op >> 4) & 0x0f;
    uint8_t lo = op & 0x0f;
    bool imm = (hi == 0x8) || (hi == 0xC);
    uint16_t ea = 0;
    if (!imm) {
        switch (hi) { case 0x9: case 0xD: ea = ea_direct(); break; case 0xA: case 0xE: ea = ea_indexed(); break; case 0xB: case 0xF: ea = ea_extended(); break; }
    }

    switch (lo) {
        case 0xC: if (imm) op_cmpu(ea_imm16()); else op_cmpu(read16(ea)); break;
        case 0xE: if (imm) op_cmps(ea_imm16()); else op_cmps(read16(ea)); break;
        default: break;
    }
}

void (CPU6809::*CPU6809::opcode_table[MAX_OPCODE])() = {
    &CPU6809::dispatch_mem_ref, nullptr, nullptr, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, nullptr, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, nullptr, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref,
    &CPU6809::page_2, &CPU6809::page_3, &CPU6809::op_nop, &CPU6809::op_sync, nullptr, nullptr, &CPU6809::op_lbra, &CPU6809::op_lbsr, nullptr, &CPU6809::op_daa, &CPU6809::op_orcc, nullptr, &CPU6809::op_andcc, &CPU6809::op_sex, &CPU6809::op_exg, &CPU6809::op_tfr,
    &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch, &CPU6809::dispatch_branch,
    &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f, &CPU6809::dispatch_30_3f,
    &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a, &CPU6809::dispatch_inherent_a,
    &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b, &CPU6809::dispatch_inherent_b,
    &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref,
    &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref, &CPU6809::dispatch_mem_ref,
    &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a,
    &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a,
    &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a,
    &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a, &CPU6809::dispatch_alu_a,
    &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b,
    &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b,
    &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b,
    &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b, &CPU6809::dispatch_alu_b,
};

#ifdef CPU6809_DEBUG
static void trace_regs(uint8_t cc, uint16_t d, uint16_t x, uint16_t y, uint16_t u, uint16_t s, uint16_t pc) {
    fprintf(stderr, "  D=%04X X=%04X Y=%04X U=%04X S=%04X CC=%02X [%c%c%c%c%c%c%c%c]\n",
        d, x, y, u, s, cc,
        (cc & 0x80) ? 'E' : '-', (cc & 0x40) ? 'F' : '-', (cc & 0x20) ? 'H' : '-',
        (cc & 0x10) ? 'I' : '-', (cc & 0x08) ? 'N' : '-', (cc & 0x04) ? 'Z' : '-',
        (cc & 0x02) ? 'V' : '-', (cc & 0x01) ? 'C' : '-');
}

static const char* am_suffix(uint8_t ir) {
    uint8_t hi = ir >> 4;
    switch (hi) {
        case 0x0: case 0x9: case 0xD: return "  [dir]";
        case 0x2: return "  [rel]";
        case 0x6: case 0xA: case 0xE: return "  [idx]";
        case 0x7: case 0xB: case 0xF: return "  [ext]";
        case 0x8: case 0xC: return "  #imm";
        default:  return "";
    }
}

#endif

void CPU6809::run_cycles(uint32_t c) {
    (void)c;
    if (reset) {
        regs_16[reg_pc] = read16(0xfffe);
        reset = false;
#ifdef CPU6809_DEBUG
        fprintf(stderr, "\n=== RESET -> PC=%04X ===\n\n", regs_16[reg_pc]);
#endif
    } else {
        if (m_irq && !(cc & CC_I)) {
            m_irq = false;
            cc |= CC_E;
            spush16(regs_16[reg_pc]);
            spush16(regs_16[reg_u]);
            spush16(regs_16[reg_y]);
            spush16(regs_16[reg_x]);
            spush(regs_16[reg_d] >> 8);
            spush(regs_16[reg_d] & 0xff);
            spush(dp);
            spush(cc);
            cc |= CC_I | CC_F;
            regs_16[reg_pc] = read16(0xfff8);
#ifdef CPU6809_DEBUG
            fprintf(stderr, "\n=== IRQ -> PC=%04X ===\n\n", regs_16[reg_pc]);
#endif
            return;
        }

        ir = read_mem(regs_16[reg_pc]++);

#ifdef CPU6809_DEBUG
        uint16_t save_pc = regs_16[reg_pc] - 1;
        const char* mn = mnemonic_table[ir];
        if (mn) {
            fprintf(stderr, "%04X: %02X %-6s%s %x ", save_pc, ir, mn, am_suffix(ir), read_mem(regs_16[reg_pc]));
        } else if (ir >= 0x21 && ir <= 0x2F) {
            fprintf(stderr, "%04X: %02X %-6s  [rel]", save_pc, ir, "???");
        } else {
            fprintf(stderr, "%04X: %02X %-6s", save_pc, ir, "???");
        }

        switch (ir >> 4) {
            case 0x0: { uint8_t b = ir & 0x0f ? read_mem(regs_16[reg_pc]) : 0; fprintf(stderr, " $%02X", dp); fprintf(stderr, "%02X", b); } break;
            case 0x2: { uint8_t b = read_mem(regs_16[reg_pc]); if (ir != 0x21) fprintf(stderr, " $%04X", save_pc + 2 + (int8_t)b); } break;
            case 0x3: if (ir == 0x34 || ir == 0x35 || ir == 0x36 || ir == 0x37 || ir == 0x3C) { fprintf(stderr, " #$%02X", read_mem(regs_16[reg_pc])); } break;
            case 0x7: case 0xB: case 0xF: { uint8_t h = read_mem(regs_16[reg_pc]), l = read_mem(regs_16[reg_pc] + 1); fprintf(stderr, " $%02X%02X", h, l); } break;
            case 0x8: case 0xC: fprintf(stderr, " #$%02X", read_mem(regs_16[reg_pc])); break;
            default: break;
        }
#endif

        void (CPU6809::*func)() = opcode_table[ir];
        if (func != nullptr) {
            (this->*func)();
        }
#ifdef CPU6809_DEBUG
        else {
            fprintf(stderr, " (UNDEF)");
        }
        trace_regs(cc, regs_16[reg_d], regs_16[reg_x], regs_16[reg_y],
                   regs_16[reg_u], regs_16[reg_s], regs_16[reg_pc]);
#endif
    }
}
