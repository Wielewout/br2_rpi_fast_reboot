CC = gcc

CC_CMD = $(CC) -o '$@' '$<' $(CFLAGS)

TARGET = shutdowncheck

.PHONY = all clear

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC_CMD)

clean:
	$(RM) $(TARGET)