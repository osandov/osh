ALL_CFLAGS := -Wall -g -std=gnu99 $(CFLAGS) 

SRCS := main.c \
	builtin.c \
	cmdline.c \
	error.c \
	parser.c \
	tokenizer.c

BUILD ?= build

OBJS := $(addprefix $(BUILD)/, $(SRCS:.c=.o))

$(BUILD)/osh: $(OBJS)
	$(CC) $(ALL_CFLAGS) -o $@ $^

$(BUILD)/%.o : %.c | $(BUILD)
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm $(OBJS) $(BUILD)/osh
	rmdir $(BUILD)
