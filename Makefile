SRCS := $(shell find . -name '*.c')
OBJS := $(SRCS:.c=.o)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

fur: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fur
