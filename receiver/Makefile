CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses -pthread

OBJS = main.o state.o threads.o
TARGET = recv429

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)