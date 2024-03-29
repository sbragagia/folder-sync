CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = folder-sync.c
OBJS = $(SRCS:.c=.o)
TARGET = folder-sync

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $@ $^

%.o: %.c
    $(CC) $(CFLAGS) -c -o $@ $<

clean:
    rm -f $(TARGET) $(OBJS)
