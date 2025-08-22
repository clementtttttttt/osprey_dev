/* 
 * 2018, Laurent Ghigonis <laurent@gouloum.fr>
 */

#include "conf.h"

void ps2_init(void);
void ps2_read(void);
void ps2_buf_push(unsigned short c);
unsigned short ps2_buf_pull(void);
void ps2_buf_empty(void);
#define PS2_RELEASED 0x100
#define PS2_EXTENDED 0x200
#define PS2_EX2 0x400
