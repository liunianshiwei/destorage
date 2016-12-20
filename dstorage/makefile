.PHONY:clean all

WORKDIR=.
VPATH = ./src
OBJDIR=./obj

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/include/  -I/usr/include/mysql/ -I/usr/local/include/hiredis/ -I/usr/include/fastdfs/ -I/usr/include/fastcommon/
LIBFLAG = -L /usr/lib

BIN = client_upload example redis_op_test fcgi_upload fcgi_data fcgi_reg fcgi_login

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
	
fcgi_main:fcgi_main.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lfcgi
	@cp	$(OBJDIR)/fcgi_main .	
	
fcgi_upload:fcgi_upload.o make_log.o util_cgi.o redis_op.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lfcgi -lhiredis -lm -lfdfsclient
	@cp	$(OBJDIR)/fcgi_upload .
	
fcgi_data:fcgi_data.o make_log.o util_cgi.o redis_op.o cJSON.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lfcgi -lhiredis -lm -lfdfsclient
	@cp	$(OBJDIR)/fcgi_data .

fcgi_reg:fcgi_reg.o make_log.o util_cgi.o redis_op.o cJSON.o dao_mysql.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lfcgi -lhiredis -lm -lfdfsclient -lmysqlclient
	@cp	$(OBJDIR)/fcgi_reg .

fcgi_login:fcgi_login.o make_log.o util_cgi.o redis_op.o cJSON.o dao_mysql.o
	cd	$(OBJDIR);$(CC) $(LIBFLAG) $^ -o $@ -lfcgi -lhiredis -lm -lfdfsclient -lmysqlclient
	@cp	$(OBJDIR)/fcgi_login .	

	
%.o:%.c
	$(CC) $(CFLGS) -c $< -o $(OBJDIR)/$(@F)
	
clean:
	-rm -f $(OBJDIR)/* $(BIN)
