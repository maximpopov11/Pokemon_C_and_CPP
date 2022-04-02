pokemon: main.cpp
	g++ main.cpp heap.c -g -Wall -o pokemon -lm -lncurses

clean:
	rm -f pokemon