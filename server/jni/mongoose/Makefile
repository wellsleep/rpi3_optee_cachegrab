CC = gcc
CFLAGS := -Wall -g

SRC = mongoose.c
OBJS = mongoose.o
LIB = libmongoose.a

all : $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -I.

$(LIB) : $(OBJS)
	rm -f $@
	ar rcs $@ $(OBJS)
	#rm -f $(OBJS)
