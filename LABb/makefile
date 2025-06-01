
all: AntiVirus

AntiVirus: AntiVirus.o
		gcc -m32 -g -o AntiVirus AntiVirus.o

AntiVirus.o: AntiVirus.c
		gcc -g -m32 -c -o AntiVirus.o AntiVirus.c

.PHONY: clean

clean:
		rm -f *.o AntiVirus