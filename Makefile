CC=gcc
CFLAGS=-Wall -g
MAIN=main
OBJECTS=main.o mem_pool.o

all: $(MAIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN): $(OBJECTS)
	$(CC) $^ -o $@

.PHONY: clean
clean:
	rm -f $(MAIN) $(OBJECTS)