CC=gcc
CFLAGS=-std=c11 -Wall -Werror -march=native -O3
LDFLAGS=-lm

mushroom: main.o
	${CC} ${LDFLAGS} -o mushroom main.o

