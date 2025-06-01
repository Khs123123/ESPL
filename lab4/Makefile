# all: hexeditplus task4        # Rule: make all

# hexeditplus: task1.c
# 	gcc -m32 -g -Wall -o hexeditplus task1.c

# task4: task4.c
# 	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c

# clean:
# 	rm -f *.o hexeditplus task4
all: hexeditplus task4 task4.o

hexeditplus: task1.c
	gcc -m32 -g -Wall -o hexeditplus task1.c

task4: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c

task4.o: task4.c
	gcc -m32 -fno-pie -fno-stack-protector -c task4.c -o task4.o

clean:
	rm -f *.o hexeditplus task4