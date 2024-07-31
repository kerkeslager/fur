CFLAGS = -Wall -Wextra -Wimplicit-fallthrough=5 -I/usr/local/include -L/usr/local/lib -lreadline
SRCS := $(shell find src -name '*.c')
HEADERS := $(shell find src -name '*.h')
OBJS := $(patsubst src/%.c, obj/%.o, $(SRCS))
TEST_SRCS := $(shell find src -name '*.c' -not -name main.c)
TEST_OBJS := $(patsubst src/%.c, obj/%_test.o, $(TEST_SRCS))

bin/ :
	mkdir -p bin

gen/ :
	mkdir -p gen

obj/ :
	mkdir -p obj

obj/%.o : src/%.c obj/
	$(CC) -c $(CFLAGS) $< -o $@

obj/%_test.o : src/%.c obj/
	$(CC) -c -DTEST $(CFLAGS) $< -o $@

bin/fur: $(OBJS) $(HEADERS) bin/
	$(CC) $(CFLAGS) $(OBJS) -o bin/fur

gen/unit_test.generated_c : $(HEADERS) gen/
	src/unit_test.c.sh > gen/unit_test.generated_c

obj/unit_test.generated_co: gen/unit_test.generated_c obj/
	$(CC) -c -DTEST $(CFLAGS) -x c $< -o $@

bin/unit_test: $(TEST_OBJS) $(HEADERS) obj/unit_test.generated_co bin/
	$(CC) -DTEST $(CFLAGS) $(TEST_OBJS) obj/unit_test.generated_co -o bin/unit_test

run : bin/fur
	bin/fur

test : bin/unit_test
	bin/unit_test

.PHONY: clean
clean:
	rm -rf bin gen obj
