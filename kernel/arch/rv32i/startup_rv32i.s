.section .text
.globl _start
.globl hang

_start:
  la sp, _estack
  call main

hang:
  j hang