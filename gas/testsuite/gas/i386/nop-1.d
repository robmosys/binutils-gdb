#objdump: -drw
#name: i386 .nop 1

.*: +file format .*


Disassembly of section .text:

0+ <single>:
 +[a-f0-9]+:	90                   	nop

0+1 <pseudo_1>:
 +[a-f0-9]+:	90                   	nop

0+2 <pseudo_8>:
 +[a-f0-9]+:	0f 1f 84 00 00 00 00 00 	nopl   0x0\(%eax,%eax,1\)

0+a <pseudo_8_4>:
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%eax\)
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%eax\)

0+12 <pseudo_20>:
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	nopw   %cs:0x0\(%eax,%eax,1\)
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	nopw   %cs:0x0\(%eax,%eax,1\)

0+26 <pseudo_30>:
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	nopw   %cs:0x0\(%eax,%eax,1\)
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	nopw   %cs:0x0\(%eax,%eax,1\)
 +[a-f0-9]+:	66 2e 0f 1f 84 00 00 00 00 00 	nopw   %cs:0x0\(%eax,%eax,1\)
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
#pass
