#!/bin/bash
nasm -f elf64 ${1}.asm -o ${1}.o -g
ld -e _main ${1}.o -o ${1}.out -g 
#sudo gdb ${1}.out
#./${1}.out
