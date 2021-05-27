CC = gcc

CC_CMD = $(CC) -o '$@' '$<' $(CFLAGS)

PRINT_TARGET = print-measurements
CLEAR_TARGET = clear-measurements

.PHONY = all clean

all: $(PRINT_TARGET) $(CLEAR_TARGET)

$(PRINT_TARGET): $(PRINT_TARGET).c
	$(CC_CMD)

$(CLEAR_TARGET): $(CLEAR_TARGET).c
	$(CC_CMD)

clean:
	$(RM) $(PRINT_TARGET) $(CLEAR_TARGET)