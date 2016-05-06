CFLAGS=-Wall -Wextra -pedantic -std=gnu99 -O2 -s

all: leekspeak

%: %.c
	gcc ${CFLAGS} -o $@ $^

clean:
	-rm leekspeak
