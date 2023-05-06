CFLAGS = -Wall -Wextra
SRCS := $(shell find . -name '*.c' -not -name unit_test.c)
OBJS := $(SRCS:.c=.o)
TEST_SRCS := $(shell find . -name '*.c' -not -name main.c) unit_test.c
TEST_OBJS := $(TEST_SRCS:.c=_test.o)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

%_test.o : %.c
	$(CC) -c -DTEST $(CFLAGS) $< -o $@

fur: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fur

unit_test.c :
	./unit_test.c.sh > unit_test.c

unit_test: $(TEST_OBJS)
	$(CC) -DTEST $(CFLAGS) $(TEST_OBJS) -o unit_test

.PHONY: clean
clean:
	rm -f *.o fur unit_test unit_test.c
