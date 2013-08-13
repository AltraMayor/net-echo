CC = gcc
CFLAGS = -Wall -Wextra -g -MMD
LDFLAGS = -g

TARGETS = tclient

all : $(TARGETS)

tclient : tclient.o sockutil.o
	$(CC) -o $@ $^ $(LDFLAGS)

PHONY : clean
clean :
	rm -f *.o *.d $(TARGETS)
