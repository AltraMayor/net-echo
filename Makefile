all : eserv ecli eclicork

eserv : eserv.c eutils.c
	gcc -o eserv -Wall eserv.c eutils.c

ecli : ecli.c eutils.c
	gcc -o ecli -Wall ecli.c eutils.c

eclicork : eclicork.c eutils.c
	gcc -o eclicork -Wall eclicork.c eutils.c

clean :
	rm -f eserv ecli eclicork

cscope :
	cscope -b *.c *.h
