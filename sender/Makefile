CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses

# List of the compiled object files required
OBJS = main.o flight_state.o physics.o render.o
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