CFLAGS = -Wall -Wextra -Werror -pedantic -march=native
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all
all: CFLAGS += -O3
all: ec

.PHONY: debug
debug: CFLAGS += -g -fsanitize=address -fsanitize=undefined
debug: ec

ec: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: test
test: ec
	sh tests/run_tests.sh

.PHONY: clean
clean:
	rm -rf $(OBJS) ec *.dSYM
