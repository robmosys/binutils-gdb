#as:
#readelf: -x.rodata -wL
#name: DWARF2 12
# The am3 avr cr16 crx mn10 msp430 nds32 pru rl78 and xtensa targets do not evaluate the subtraction of symbols at assembly time
# The riscv targets do not support the subtraction of symbols.
# The tile targets require 8-byte instructions, but the test only simulates 4-byte instructions.
#not-target: am3-* avr-* cr16-* crx-* mn10*-* msp430-* nds32*-* pru-* riscv*-* rl78-* tile*-* xtensa-*


Hex dump of section '\.rodata':
  0x00000000 01 *.*

Decoded dump of debug contents of section \.debug_line:

CU: dwarf2-12\.c:
File name  *Line number  *Starting address  *View
dwarf2-12\.c  *1  *0
dwarf2-12\.c  *2  *0
dwarf2-12\.c  *3  *0  *1
dwarf2-12\.c  *3  *0x4