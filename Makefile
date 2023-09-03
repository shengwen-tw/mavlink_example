UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
  # Linux (gcc)
  CFLAGS :=
  CFLAGS += -Wno-address-of-packed-member \
            -Wno-unused-result \
            -Wno-array-bounds \
            -Wno-stringop-overread
else ifeq ($(UNAME), Darwin)
  # macOS (clang)
  CFLAGS :=
else ifeq ($(UNAME), FreeBSD)
  # FreeBSD (clang)
  CFLAGS :=
endif

CFLAGS += -O2 -Wall

CFLAGS += -I ./
CFLAGS += -I ./lib/mavlink_v2
CFLAGS += -I ./lib/mavlink_v2/common

LDFLAGS := -lpthread

SRCS := \
	main.c \
	serial.c \
	net.c \
	mavlink_parser.c \
	mavlink_publisher.c

all: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mavlink $^

clean:
	$(RM) -f mavlink

format:
	clang-format -i *.c *.h

.PHONY: all clean format
