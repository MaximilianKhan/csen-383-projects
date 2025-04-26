CC = gcc
CFLAGS = -pthread
TARGET = tickets
SRC = tickets.c
OUTPUT_FILE = output.txt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

run: $(TARGET)
	echo "Compiling and running $(TARGET)..." > $(OUTPUT_FILE) 2>&1
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) >> $(OUTPUT_FILE) 2>&1
	echo "Running with N=5:" >> $(OUTPUT_FILE) 2>&1
	./$(TARGET) 5 >> $(OUTPUT_FILE) 2>&1
	echo "\nRunning with N=10:" >> $(OUTPUT_FILE) 2>&1
	./$(TARGET) 10 >> $(OUTPUT_FILE) 2>&1
	echo "\nRunning with N=15:" >> $(OUTPUT_FILE) 2>&1
	./$(TARGET) 15 >> $(OUTPUT_FILE) 2>&1

clean:
	rm -f $(TARGET) $(OUTPUT_FILE)