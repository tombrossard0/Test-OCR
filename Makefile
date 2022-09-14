# Makefile

CPPFLAGS =
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS =
LDLIBS =

all: main run clean

main: main.o Image.o

.PHONY: clean

clean:
	${RM} main.o   # remove object files
	${RM} Image.o   # remove dependency files
	${RM} main     # remove main program

run:
	$../main "Images/OCR1.png"

# END