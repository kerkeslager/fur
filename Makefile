SRCS := $(shell find . -name '*.c' -not -name unittest.c)
OBJS := $(SRCS:.c=.o)
TEST_SRCS := $(shell find . -name '*.c' -not -name main.c)
TEST_OBJS := $(TEST_SRCS:.c=_test.o)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

%_test.o : %.c unittest.c
	$(CC) -c -DTEST $(CFLAGS) $< -o $@

fur: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fur

unittest.c :
	./unittest.c.sh > unittest.c

unittest: $(TEST_OBJS)
	$(CC) -DTEST $(CFLAGS) $(TEST_OBJS) -o unittest

.PHONY: clean
clean:
	rm *.o fur unittest unittest.c
