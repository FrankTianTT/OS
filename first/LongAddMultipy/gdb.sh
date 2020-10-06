#!/bin/bash
nasm -f macho64 ${1}.asm -o ${1}.out -g
#gcc  ${1}.out -g -o ${1}.o
#sudo gdb ${1}.o