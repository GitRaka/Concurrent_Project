# Author: Rakesh Kumar
all:	bst

bst:	main.c handler.c bst_lock.c
		g++ -I. -o bst main.c handler.c bst_lock.c -pthread

clean:
	rm -f bst *.o