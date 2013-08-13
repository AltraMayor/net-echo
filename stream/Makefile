CC = gcc
CFLAGS = -Wall -Wextra -g -MMD
LDFLAGS = -g

TARGETS = tserver tclient

all : $(TARGETS)

tserver : tserver.o sockutil.o
	$(CC) -o $@ $^ $(LDFLAGS)

tclient : tclient.o sockutil.o
	$(CC) -o $@ $^ $(LDFLAGS)

PHONY : clean
clean :
	rm -f *.o *.d $(TARGETS)
