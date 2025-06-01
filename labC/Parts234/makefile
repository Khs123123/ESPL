
all: myshell looper

myshell: looper myshell.o LineParser.o
	gcc -m32 -g -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -g  -m32 -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g  -m32 -c -o LineParser.o LineParser.c

looper: looper.o 
	gcc -m32 -g -o looper looper.o 

looper.o: looper.c
	gcc -g  -m32 -c -o looper.o looper.c

.PHONY: clean 

 #Clean the build directory 

clean:
	rm -f *.o myshell looper 