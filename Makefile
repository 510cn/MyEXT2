CC = gcc
CFLAGS = -Wall -g

all: filesystem

filesystem: main.c filesystem.c filesystem.h
    $(CC) $(CFLAGS) -o filesystem main.c filesystem.c

clean:
    rm -f filesystem disk.img