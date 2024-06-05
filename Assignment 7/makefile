all: multi
multi: multi.o
	gcc -m32 -g -Wall -o multi multi.o

multi.o: multi.s
	nasm -g -f elf32 -o multi.o multi.s

.PHONY: clean

clean:
	rm -f *.o multi