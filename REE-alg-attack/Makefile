OBJS = ree_func.o

CFLAGS += -Wall 
LDADD += -lree_shim -L. -Wl,-rpath,'$$ORIGIN' 

BINARY = ree_program

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)
