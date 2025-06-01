all: myshell mypipe looper

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

mypipe: mypipe.o
	gcc -g -m32 -Wall -o mypipe mypipe.o 

looper: looper.o
	gcc -g -m32 -Wall -o looper looper.o

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c 

mypipe.o: mypipe.c
	gcc -m32 -g -Wall -c -o mypipe.o mypipe.c 

looper.o: looper.c
	gcc -m32 -g -Wall -c -o looper.o looper.c 

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c 

.PHONY: clean 

clean:
	rm -f *.o myshell mypipe looper
