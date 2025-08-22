# Copyright (c) 2012 - 2017 Thomas Buck <xythobuz@xythobuz.de>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MCU = atmega16
F_CPU = 16000000
RM = rm -rf
OPT = 3
CSTANDARD = gnu99

ISPPORT = usb
ISPTYPE = avrisp2

DOXYGEN = /Applications/Doxygen.app/Contents/Resources/doxygen

# -------------[<32;54;11MOA----------------

CARGS = -mmcu=$(MCU)
CARGS += -O$(OPT)
CARGS += -funsigned-char
CARGS += -funsigned-bitfields
CARGS += -fpack-struct
CARGS += -fshort-enums
CARGS += -ffunction-sections
CARGS += -Wall -Wstrict-prototypes
CARGS += -std=$(CSTANDARD)
CARGS += -DF_CPU=$(F_CPU) -g
LDARGS = -Wl,-L.,-lm,-lavrSerial,--relax,--gc-sections

all: sio.hex

doc: serial.c serial.h serial_device.h test.c ILI9488.h ILI9488.cpp
	$(DOXYGEN) Doxyfile
	make -C doc/latex/

flash: sio.hex
	./flash.sh

sio.hex: test.elf
	avr-objcopy -O ihex $< $@

test.elf: libavrSerial.a test.o ps2.o
	avr-gcc $(CARGS) test.o ps2.o --output test.elf $(LDARGS)
	avr-size test.elf

lib: libavrSerial.a sizelibafter

sizelibafter:
	avr-size libavrSerial.a

libavrSerial.a: serial.o
	avr-ar -c -r -s libavrSerial.a serial.o

serial.o: serial.h serial_device.h 
%.o: %.c
	avr-gcc -c $< -o $@ $(CARGS)
%.o: %.cpp
	avr-g++ -c $< -o $@ $(CARGS)
clean:
	$(RM) *.o
	$(RM) *.a
	$(RM) *.elf
	$(RM) *.hex

