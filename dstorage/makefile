.PHONY:clean all

WORKDIR=.
VPATH = ./test

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/include/
LIBFLAG = -L /usr/lib

BIN = client_upload

all:$(BIN)

client_upload:make_log.o client_upload.o
	$(CC) $(LIBFLAG) $^ -o $@ 
%.o:%.c
	$(CC) $(CFLGS) -c $< -o $@
	
clean:
	rm -f *.o $(BIN)
	
	
	




