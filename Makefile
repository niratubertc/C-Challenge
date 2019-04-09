cchallenge:
	gcc -O3 -o cchallenge main.c

clean:
	rm -f cchallenge

all: cchallenge

.PHONY: all clean
