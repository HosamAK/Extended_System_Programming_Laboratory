all: hexeditplus nntsc

nntsc: task4.c
	gcc -m32 -fno-pie -fno-stack-protector -o nntsc task4.c

hexeditplus: task1.c
	gcc -m32 -g -Wall -o hexeditplus task1.c

.PHONY: clean

clean:
	rm -f hexeditplus nntsc