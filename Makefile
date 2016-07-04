CFLAGS=-Wall -Wextra -pedantic -std=gnu99 -O2 -s

all: leekspeak

leekspeak: leekspeak.c onion.c
	gcc ${CFLAGS} -o $@ $^

clean:
	-rm leekspeak
