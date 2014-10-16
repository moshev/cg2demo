CC=gcc
CFLAGS=-std=c11 -Wall -Werror -march=native -Og -g
LDFLAGS=-lm

texttest: texttest.o
	${CC} ${LDFLAGS} -o texttest texttest.o

