CFLAGS += -Wall
debug: CFLAGS += -g
CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)
CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

BINARY = hsmRadioBrowser

INCLUDES = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

help:
	@echo
	@echo Please specify a target:
	@echo
	@echo "$$ make <target>"
	@echo
	@echo "Valid targets are 'clean, debug' or 'release'"
	@echo

debug: $(BINARY)
release: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *.o *~ $(BINARY)

.PHONY: clean
