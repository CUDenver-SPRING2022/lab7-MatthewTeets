# Makefile for lab7 server/client program

CC = gcc
OBJCSS = server_client7.c

CFLAGS = -g -Wall -lm

#setup for system
nLIBS = 

all: client7

client7: $(OBJCSS)
	$(CC) $(CFLAGS) -o $@ $(OBJCSS) $(LIBS)

clean:
	rm client7
