#ifndef __ATMEGA328P_
#define __ATMEGA328P_

#include <stdint.h>
#include <stdbool.h>

#include "interrupts.h"

typedef uint8_t byte;
typedef uint8_t bit;
typedef uint16_t word;

#define WORD_SIZE sizeof(word)
#define REGISTER_COUNT 32
#define IO_REGISTER_COUNT 64
#define EXT_IO_REGISTER_COUNT 0
#define PROGRAM_MEMORY_SIZE (16 * KB)
#define BOOTLOADER_SIZE (KB / 2)
#define RAM_SIZE (1 * KB)
#define DATA_MEMORY_SIZE (REGISTER_COUNT + IO_REGISTER_COUNT + EXT_IO_REGISTER_COUNT + RAM_SIZE)
#define KB 1024
#define LOOKUP_SIZE 0xFFFF

typedef union {
  // Status register flags
  struct {
    bit C : 1; // Carry
    bit Z : 1; // Zero
    bit N : 1; // Negative
    bit V : 1; // Overflow
    bit S : 1; // Sign
    bit H : 1; // Half carry
    bit T : 1; // Copy storage
    bit I : 1; // Interrupt enable
  } flags __attribute__((packed));
  byte value;
} SREG_t;

typedef union {
  // MCU status register flags
  struct {
    bit reserved4 : 1;
    bit reserved3 : 1;
    bit reserved2 : 1;
    bit reserved1 : 1;
    // reset flags
    bit WDRF : 1; // Watchdog
    bit BORF : 1; // Brown-out
    bit EXTRF : 1; // External
    bit PORF : 1; // Power-on
  } flags;
  byte value;
} MCUSR_t;
static inline uint8_t io_read(uint8_t a);

typedef struct {
  char *name;
  void (*execute)(uint32_t opcode);
  uint16_t mask1; // 1 for all fixed bits, 0 for variables
  uint16_t mask2; // 1 for all fixed 1s, 0 for all fixed 0s and variables
  uint16_t cycles;
  uint16_t length; // in WORDs
} Instruction_t;

typedef struct {
  SREG_t *SREG;
  MCUSR_t SR; // MCU status register
  byte data_memory[DATA_MEMORY_SIZE]; // contains registers and RAM, allows various addressing modes
  byte ROM[KB];
  byte program_memory[PROGRAM_MEMORY_SIZE];
  byte *boot_section; // Last 512 bytes of program memory
  byte *R; // General purpose registers
  byte *IO; // IO registers
  byte *ext_IO; // External IO registers
  byte *RAM;
  uint16_t *sp; // Stack pointer, 2 bytes needed to address the 2KB RAM space
  uint16_t pc; // Program counter
  bool skip_next;
  bool sleeping;
  bool stopped;
  bool handle_interrupt;
  bool auto_execute;
  uint16_t interrupt_address;
  int16_t data_memory_change; // -1 if there was no change, data_memory address otherwise
  uint16_t cycles;
  uint32_t opcode;
  const Instruction_t *instruction;
  void (*exception_handler)(void);
} ATmega16a_t;

// API
void mcu_init(void);
bool mcu_load_ihex(const char *filename);
bool mcu_load_asm(const char *code);
bool mcu_load_c(const char *code);
void mcu_run(void);
bool mcu_execute_cycle(void);
void mcu_resume(void);
void mcu_get_copy(ATmega16a_t *mcu);
void mcu_send_interrupt(Interrupt_vector_t vector);
void mcu_set_exception_handler(void (*handler)(void));

static inline void execute_instruction(void);
static inline void set_mcu_pointers(ATmega16a_t *const mcu);
static void create_lookup_table(void);
static const Instruction_t *find_instruction(const uint16_t opcode);
static inline uint16_t get_opcode16(void);
static inline uint32_t get_opcode32(void);
static inline bool check_interrupts(void);
static inline void handle_interrupt(void);

static inline void stack_push16(const uint16_t value);
static inline void stack_push8(const uint8_t value);
static inline uint16_t stack_pop16(void);
static inline uint8_t stack_pop8(void);

static inline uint16_t word_reg_get(const uint8_t d);
static inline void word_reg_set(const uint8_t d, const uint16_t value);
static inline uint16_t X_reg_get(void);
static inline uint16_t Y_reg_get(void);
static inline uint16_t Z_reg_get(void);
static inline void X_reg_set(const uint16_t value);
static inline void Y_reg_set(const uint16_t value);
static inline void Z_reg_set(const uint16_t value);

static inline uint64_t get_micro_time(void);
static inline void throw_exception(const char *cause, ...);

#endif // __ATMEGA328P_
