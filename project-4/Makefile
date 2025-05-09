CC = gcc

# List of source files (case-sensitive!)
SRCS = Page.c FIFO.c LFU.c LRU.c MFU.c Random.c main.c
OBJS = $(SRCS:.c=.o)
EXEC = main

.PHONY: all clean run

# Build target
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) -c $< -o $@

# Clean up build files and logs
clean:
	rm -f $(OBJS) $(EXEC)
	rm -f logs_*.txt

# Run all algorithms one by one
run: clean all
	./$(EXEC) FIFO
	./$(EXEC) LFU
	./$(EXEC) LRU
	./$(EXEC) MFU
	./$(EXEC) Random
