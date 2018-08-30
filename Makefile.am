AUTOMAKE_OPTIONS = foreign

SUBDIRS = gwlib wmlscript radius wap gw utils test doc checks benchmarks

# TODO:
# build gw-config
# install test (conditionally)
# test soap
# test doc


EXTRA_DIST = AUTHORS ChangeLog COPYING INSTALL LICENSE NEWS README STATUS VERSION contrib 

nag:
	utils/find-long-lines

pkg: all
	(cd solaris; mk-solaris-package.sh)
	echo Package is now in solaris directory

gw-config: utils/foobar-config.sh Makefile
	./utils/foobar-config.sh "-I$(includedir)/kannel @CFLAGS@" \
		"-L$(libdir)/kannel -lgw -lwap -lgwlib $(LIBS)" \
		"@VERSION@" > gw-config
	chmod 0755 gw-config

dist-hook:
	rm -rf `find $(distdir) -name CVS -or -name SCCS or -name .svn`

