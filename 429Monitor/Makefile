CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE
LIBS = -lncurses -lutil

TARGET = dash_monitor
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
