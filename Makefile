SRCS := $(shell find . -name '*.c' -not -name unittest.c)
OBJS := $(SRCS:.c=.o)
TEST_SRCS := $(shell find . -name '*.c' -not -name main.c)
TEST_OBJS := $(TEST_SRCS:.c=_test.o)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

%_test.o : %.c
	$(CC) -c -DTEST $(CFLAGS) $< -o $@

fur: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fur

unittest: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o unittest
