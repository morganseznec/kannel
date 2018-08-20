AM_CFLAGS = -I. -I$(top_builddir)/gw -I$(top_builddir)/gwlib -I$(top_builddir)

lib_LIBRARIES = libwmlscript.a
bin_PROGRAMS = wmlsc wmlsdasm
# -v gives verbose output.
AM_YFLAGS = -d -p ws_yy_

wmlsc_LDADD = libwmlscript.a $(top_builddir)/libgwlib.a
wmlsc_SOURCES = wmlsc.c

wmlsdasm_LDADD = libwmlscript.a $(top_builddir)/libgwlib.a
wmlsdasm_SOURCES = wmlsdasm.c

libwmlscript_a_SOURCES = \
	ws.c \
	ws.h \
	wsalloc.c \
	wsalloc.h \
	wsasm.c \
	wsasm.h \
	wsbc.c \
	wsbc.h \
	wsbuffer.c \
	wsbuffer.h \
	wsencode.c \
	wsencode.h \
	wserror.c \
	wserror.h \
	wsfalloc.c \
	wsfalloc.h \
	wsgram.y \
	wshash.c \
	wshash.h \
	wsieee754.c \
	wsieee754.h \
	wsint.h \
	wslexer.c \
	wsopcodes.h \
	wsopt.c \
	wsopt.h \
	wsstdlib.c \
	wsstdlib.h \
	wsstream.c \
	wsstream.h \
	wsstream_data.c \
	wsstream_file.c \
	wsstree.c \
	wsstree.h \
	wsutf8.c \
	wsutf8.h


BUILT_SOURCES = wsopcodes.h wsgram.h wsgram.c

dist_noinst_SCRIPTS = make-op-table
dist_noinst_DATA = lexer.txt

make-op-table: make-op-table.in
	sed 's%@PERLPROG@%@PERL@%g' make-op-table.in \
		> make-op-table
	chmod a+x make-op-table

opcodes:
	./make-op-table wsasm.h \
		> wsopcodes.h

wsopcodes.h: make-op-table wsasm.h
	$(PERL) make-op-table wsasm.h \
		> wsopcodes.h

