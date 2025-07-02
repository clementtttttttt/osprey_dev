#ifndef CPUATMEGA16_H
#define CPUATMEGA16_H

#include <cstdint>

#include <string>
//sio defaults
#define FLASH_SZ 0x2000
#define PROGMEM_SZ 0x1f80
#define BOOT_SECTION_SZ FLASH_SZ-PROGMEM_SZ
//NOTE: PROGRAM MEMORY IN 16 bits words

#define SRAM_SZ 1024
#define IO_REGS 0x40
#define REGS 0x20



class CPUAtmega16
{

    typedef union{
        uint8_t val;
        struct{
            uint8_t c:1;
            uint8_t z:1;
            uint8_t n:1;
            uint8_t v:1;
            uint8_t s:1;
            uint8_t h:1;
            uint8_t t:1;
            uint8_t i:1;

        };

    } SREG_t;
    public:
        CPUAtmega16();
        virtual ~CPUAtmega16();
        void cycle(uint32_t cycles);
        void loadHex(std::string);
    protected:

    private:
        uint16_t flash[FLASH_SZ];
        uint8_t data[SRAM_SZ + IO_REGS + REGS];
        uint8_t *regs = &data[0];
        uint8_t *io_regs = &data[REGS];
        uint8_t *sram = &data[IO_REGS + REGS];
        SREG_t *sreg = reinterpret_cast<SREG_t*>(&io_regs[0x3f]);
        uint16_t *sp = reinterpret_cast<uint16_t*>(&io_regs[0x3d]);
        uint16_t pc;

        static void(CPUAtmega16::* opcode_funs[])(uint16_t)  ;

        uint8_t getRegr(uint16_t);
        uint8_t getRegd(uint16_t);
        uint8_t get4bitRegd(uint16_t);
        uint8_t getSubcode(uint16_t);
        uint8_t isTwoBytes(uint16_t);
        uint8_t setSubSreg(uint8_t rd, uint8_t rr, uint8_t c);
        uint8_t setAddSreg(uint8_t rd, uint8_t rr, uint8_t c);
        void setBitopSreg(uint8_t in);
        void skipNextInstruction();
        uint8_t getImmediate8(uint16_t in);
        short getBranchingOffset(uint16_t in);

        void push8(uint8_t in);
        uint8_t pop8();
        void push16(uint16_t);
        uint16_t pop16();

        void GRP0(uint16_t);
        void GRP1(uint16_t);
        void GRP2(uint16_t);
        void LDD(uint16_t);
        void GRP4(uint16_t);
        void CPI(uint16_t);
        void SBCI(uint16_t);
        void SUBI(uint16_t);
        void ORI(uint16_t);
        void ANDI(uint16_t);
        void INOUT(uint16_t);
        void RJMP(uint16_t);
        void RCALL(uint16_t);
        void LDI(uint16_t);
        void GRPF(uint16_t);

        uint8_t readIO(uint16_t);
        uint8_t readData(uint16_t);
        void writeIO(uint16_t, uint8_t);
        void writeData(uint16_t, uint8_t);


};

#endif // CPUATMEGA16_H
