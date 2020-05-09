CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -g

default: build

build:
	${CC} ${CFLAGS} -o tema3 html.c interpret.c commands.c parser.c struct.c

clean:
	rm -f tema3
