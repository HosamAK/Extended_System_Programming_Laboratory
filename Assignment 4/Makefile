all: myshell mypipe

mypipe: mypipe.o
	gcc -m32 -g -Wall -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -m32 -g -Wall -c -o mypipe.o mypipe.c

myshell: myshell.o LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

.PHONY: clean

clean:
	rm -f *.o myshell mypipe
