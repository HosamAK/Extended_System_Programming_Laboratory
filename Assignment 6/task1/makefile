all: start
start: util.o
	nasm -f elf32 start.s -o start.o
	gcc -m32 util.o start.o -o start

util.o:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

.PHONY: clean
clean:
	rm -f *.o start