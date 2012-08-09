all : eserv ecli eclicork

eserv : eserv.c eutils.c
	gcc -o eserv eserv.c eutils.c

ecli : ecli.c eutils.c
	gcc -o ecli ecli.c eutils.c

eclicork : eclicork.c eutils.c
	gcc -o eclicork eclicork.c eutils.c

clean :
	rm -f eserv ecli eclicork
