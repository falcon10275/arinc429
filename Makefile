# Root Makefile

.PHONY: all sender receiver clean

# Default target runs both
all: sender receiver

sender:
	$(MAKE) -C sender
	cp sender/send429 .

receiver:
	$(MAKE) -C receiver
	cp receiver/recv429 .

# Clean target to remove generated files
clean:
	$(MAKE) -C sender clean
	$(MAKE) -C receiver clean
	rm -f send429 recv429