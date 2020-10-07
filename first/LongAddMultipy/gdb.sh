#!/bin/bash
nasm -f elf64 ${1}.asm -o ${1}.out -g
ld -e _main ${1}.out -o ${1}.o -g 
#sudo gdb ${1}.o
