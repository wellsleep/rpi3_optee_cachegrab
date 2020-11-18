CC = gcc
CFLAGS := -Wall -lz -g

SRC = server.c server_scope.c server_probe.c server_capture.c \
      scope.c capture.c capture_data.c \
      thread_utils.c thread_scope.c thread_target.c thread_stall.c
OBJS = $(patsubst ./%c, ./%.o, $(SRC))
INC = -I../libpng -I../mongoose -I.
VPATH = ../libpng:../mongoose
TARGET = cachegrab_optee_server


all : $(TARGET)
	
%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ -L. -lmongoose -lpthread

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(INC) -L. -lpng16 -lmongoose -lpthread -D _DEFAULT_SOURCE -lz -lm -o $(TARGET)
