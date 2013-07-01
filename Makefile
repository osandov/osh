CC ?= gcc
CFLAGS ?= -Wall -g -std=gnu99

SRCS := main.c \
	builtin.c \
	parser.c \
	tokenizer.c

BUILD ?= build

OBJS := $(addprefix $(BUILD)/, $(SRCS:.c=.o))

$(BUILD)/osh: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o : %.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm $(OBJS) $(BUILD)/osh
	rmdir $(BUILD)
