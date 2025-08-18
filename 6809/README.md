# OSPREY operating system 

A keyboard-only graphical interface operating system, for the OSPREY-68 computer. 


| ADDR\VIA PAGESEL | C000-FFFF          | 8000-BFFF                  | 0-7FFF |
|------------------|--------------------|----------------------------|--------|
| 0                | ROM(Low 16k)/RAM0  | IO (Mirrored every 0x1000) | LOWRAM |
| 1                | ROM(High 16k)/RAM1 | IO (Mirrored every 0x1000) | LOWRAM |
| 2                | RAM2               | IO (Mirrored every 0x1000) | LOWRAM |
| 3                | RAM3               | IO (Mirrored every 0x1000) | LOWRAM |
| 4                | RAM4               | IO (Mirrored every 0x1000) | LOWRAM |
| 5                | RAM5               | IO (Mirrored every 0x1000) | LOWRAM |
| 6                | RAM6               | IO (Mirrored every 0x1000) | LOWRAM |

