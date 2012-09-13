all : eserv ecli eclicork

eserv : eserv.c eutils.c
	gcc -o eserv -Wall -I kernel eserv.c eutils.c dag.c ppal_map.c

ecli : ecli.c eutils.c
	gcc -o ecli -Wall -I kernel ecli.c eutils.c dag.c ppal_map.c

eclicork : eclicork.c eutils.c
	gcc -o eclicork -Wall -I kernel eclicork.c eutils.c dag.c ppal_map.c

clean :
	rm -f eserv ecli eclicork

cscope :
	cscope -b *.c *.h
