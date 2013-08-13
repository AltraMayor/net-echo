CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -I ../xiaconf/kernel-include \
-I ../xiaconf/include
LDFLAGS = -g -L ../xiaconf/libxia -lxia

TARGETS = eserv ecli eclicork

all : $(TARGETS)

eserv : eserv.o eutils.o
	$(CC) -o $@ $^ $(LDFLAGS)

ecli : ecli.o eutils.o
	$(CC) -o $@ $^ $(LDFLAGS)

eclicork : eclicork.o eutils.o
	$(CC) -o $@ $^ $(LDFLAGS)

-include *.d

PHONY : clean cscope
clean :
	rm -f *.o *.d cscope.out $(TARGETS)
cscope :
	cscope -b *.c *.h
