all: task1

task1: start.o util.o
	ld -m elf_i386 start.o util.o -o task1

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: Util.c Util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector Util.c -o util.o

clean:
	rm -f *.o task1
