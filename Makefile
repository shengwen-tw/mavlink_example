LDFLAGS=-lpthread

CFLAGS=

CFLAGS+=-Wno-address-of-packed-member

CFLAGS+=-I./
CFLAGS+=-I./lib/mavlink_v2
CFLAGS+=-I./lib/mavlink_v2/common

SRC=main.c \
	serial.c \
	net.c \
	mavlink_parser.c \
	mavlink_publisher.c

all: $(SRC)
	gcc $(CFLAGS) $(LDFLAGS) -o mavlink $^

clean:
	rm -f mavlink

format:
	clang-format -i *.c *.h

.PHONY: all clean format
