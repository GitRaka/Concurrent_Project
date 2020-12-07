# Author: Rakesh Kumar
all:	bst

bst:	main.c handler.c bst_lock.c bst_lock_rw.c
		g++ -I. -o bst main.c handler.c bst_lock.c bst_lock_rw.c -pthread

clean:
	rm -f bst *.o