CC = gcc

CC_CMD = $(CC) -o '$@' '$<' $(CFLAGS)

IDLE_TARGET = idle-measurements
SHUT_TARGET = shut-measurements

.PHONY = all clean

all: $(IDLE_TARGET) $(SHUT_TARGET)

$(IDLE_TARGET): $(IDLE_TARGET).c
	$(CC_CMD)

$(SHUT_TARGET): $(SHUT_TARGET).c
	$(CC_CMD)

clean:
	$(RM) $(IDLE_TARGET) $(SHUT_TARGET)