all: exec

exec: theELF
	gcc -g -m32 myELF.o -o myELF

theELF: myELF.c
	gcc -g -m32 -c -o myELF.o myELF.c


.PHONY:
clean:
	rm -rf  myELF