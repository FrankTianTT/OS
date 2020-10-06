#!/bin/bash
nasm -f macho64 ${1}.asm -o ${1}.out -g
gcc  ${1}.out -g -o ${1}.o
#ld -e _start ${1}.out -macosx_version_min 10.13 -lSystem -o ${1}.o
#./${1}.o