;GLOBAL VARIABLES DEFINITIONS
STACK equ 0x400 ;1024 bytes of stack, from 0x0 to 0x400


WATCHDOG_REMAINING_TICKS equ 0x401 ;decrements at 30hz pace, kill process when equal to 0x0
