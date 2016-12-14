.PHONY:clean all

WORKDIR=.
VPATH = ./test

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/include/  -I/usr/local/include/hiredis/
LIBFLAG = -L /usr/lib

BIN = client_upload example

all:$(BIN)

client_upload:make_log.o client_upload.o
	$(CC) $(LIBFLAG) $^ -o $@ 
	
example:example.o
	$(CC) $(LIBFLAG) $^ -o $@ -lhiredis
	
%.o:%.c
	$(CC) $(CFLGS) -c $< -o $@
	
clean:
	rm -f *.o $(BIN)

