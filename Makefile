testadp.o: testadp.c
	gcc -g -Wall -c testadp.c -o testadp.o

testadp: testadp.o
	gcc testadp.o -o testadp
