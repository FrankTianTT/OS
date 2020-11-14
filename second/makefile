Main: main.cpp asm_print.asm
	nasm -f elf32 asm_print.asm
	g++ -m32 main.cpp asm_print.o -o main
	rm -rf my_print.o
