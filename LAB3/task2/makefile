# all: start

# start: main.o start.o util.o
# 	gcc -m32 main.o start.o util.o -o start

# main.o: main.c util.h
# 	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

# start.o: start.s
# 	nasm -f elf32 start.s -o start.o

# util.o: util.c util.h
# 	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

# clean:
# 	rm -f *.o start

# Define compiler and assembler
CC = gcc
AS = nasm

# Define compiler and assembler flags
CFLAGS = -m32 -nostdlib -fno-builtin
ASFLAGS = -f elf32


# Define source and object files
SRCS = main.c util.c start.s
OBJS = main.o util.o start.o

# Define the output executable
OUTPUT = virus

# Default target to build the executable
all: $(OUTPUT)

# Rule to build the executable
$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to assemble the assembly files
start.o: start.s
	$(AS) $(ASFLAGS) $< -o $@

# Rule to compile the C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean the build directory
clean:
	rm -f $(OBJS) $(OUTPUT)

# Phony targets
.PHONY: all clean
