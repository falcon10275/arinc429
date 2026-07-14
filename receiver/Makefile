CC = gcc
CFLAGS = -Wall -Wextra -O2  -I../common
LDFLAGS = -lncurses -pthread

# Tell make to look in ../common for any .c files it can't find in the current directory
vpath %.c ../common

OBJS = main.o state.o threads.o arinc_429.o
TARGET = recv429

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)