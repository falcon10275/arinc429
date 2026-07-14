CC = gcc
# Added -I../common to ensure the compiler can find the corresponding .h files
CFLAGS = -Wall -Wextra -O2 -I../common
LDFLAGS = -lncurses -pthread

# Tell make to look in ../common for any .c files it can't find in the current directory
vpath %.c ../common

# List of the compiled object files required
OBJS = main.o flight_state.o physics.o render.o arinc_429.o udp_sender.o
TARGET = send429

# The default rule when you type 'make'
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up compiled files
clean:
	rm -f *.o $(TARGET)