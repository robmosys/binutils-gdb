#source: nop-3.s
#objdump: -drw
#name: x86-64 .nop 3

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	85 c0                	test   %eax,%eax
 +[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax

Disassembly of section .altinstr_replacement:

0+ <.altinstr_replacement>:
 +[a-f0-9]+:	e9 00 00 00 00       	jmpq   5 <_start\+0x5>	1: R_X86_64_PLT32	foo-0x4
#pass
