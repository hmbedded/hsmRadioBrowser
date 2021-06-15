CFLAGS += -Wall
CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)

BINARY = hsmRadioBrowser

INCLUDES = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

$(BINARY): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *.o *~ $(BINARY)

.PHONY: clean
