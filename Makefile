CFLAGS = -Wall -Wextra
SRCS := $(shell find . -name '*.c')
OBJS := $(SRCS:.c=.o)
TEST_SRCS := $(shell find . -name '*.c' -not -name main.c)
TEST_OBJS := $(TEST_SRCS:.c=_test.o)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

%_test.o : %.c
	$(CC) -c -DTEST $(CFLAGS) $< -o $@

fur: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fur

unit_test.generated_c :
	./unit_test.c.sh > unit_test.generated_c

unit_test.generated_co: unit_test.generated_c
	$(CC) -c -DTEST $(CFLAGS) -x c $< -o $@

unit_test: $(TEST_OBJS) unit_test.generated_co
	$(CC) -DTEST $(CFLAGS) $(TEST_OBJS) unit_test.generated_co -o unit_test

.PHONY: clean
clean:
	rm -f *.o *.generated_c *.generated_co fur unit_test
