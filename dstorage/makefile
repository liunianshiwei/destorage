.PHONY:clean all

WORKDIR=.
VPATH = ./src
OBJDIR=./obj

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/include/  -I/usr/local/include/hiredis/
LIBFLAG = -L /usr/lib

BIN = client_upload example redis_op_test

all:$(BIN)

client_upload:make_log.o client_upload.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@
	@cp	$(OBJDIR)/client_upload .
	
example:example.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lhiredis
	@cp	$(OBJDIR)/example .
	
redis_op_test:redis_op_test.o make_log.o redis_op.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lhiredis
	@cp	$(OBJDIR)/redis_op_test .
	
%.o:%.c
	$(CC) $(CFLGS) -c $< -o $(OBJDIR)/$(@F)
	
clean:
	-rm -f $(OBJDIR)/*.o $(BIN)
