CC = gcc
CFLAGS = -Wall -g

smallsh: smallsh.c
	$(CC) $(CFLAGS) -o smallsh smallsh.c

clean:
	rm -f smallsh