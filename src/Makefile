#test makefile
CC = gcc
TARGET = test
OBJ = engine.o main.o
CFLAGS = -Wall -Wextra -O2 -g -MMD -MP

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

engine.o: engine.c engine.h
	$(CC) $(CFLAGS) -c $< -o $@


main.o: main.c engine.h 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)


