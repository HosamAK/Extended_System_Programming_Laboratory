all: my_loader

my_loader: my_loader.o start.o
	ld -o my_loader my_loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

my_loader.o: task2.c
	gcc -m32 -Wall -c task2.c -o my_loader.o

start.o: start.s
	nasm -g -f elf32 -o start.o start.s

.PHONY: clean

clean:
	rm -f my_loader.o start.o my_loader