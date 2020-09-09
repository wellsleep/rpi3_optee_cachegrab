CC = gcc
CFLAGS := -Wall -fPIC -std=c99 -rdynamic

SRC = optee_shim.c utils.c
OBJS = optee_shim.o utils.o
LIB = libcachegrab_optee_shim.so

all : $(LIB)

.c.o:
	$(CC) $(CFLAGS) -shared -c $< -o $@ -I../server/ -L ldl

$(LIB) : $(OBJS)
	rm -f $@
	$(CC) -shared -o $(CFLAGS) -o $@ $^ -I../server/ -L/usr/lib/aarch64-linux-gnu/ -ldl
	#rm -f $(OBJS)
