AM_CFLAGS   = -Wall
AM_CPPFLAGS = -DDZ_PREFIX

include_HEADERS = \
	dzip.h \
	dzipcon.h \
	dzip-zconf.h \
	dzip-zlib.h

lib_LTLIBRARIES = libdzip.la
libdzip_la_LDFLAGS = -no-undefined
libdzip_la_SOURCES = \
	compress.c \
	conmain-lib.c \
	crc32.c \
	decode.c \
	delete.c \
	dzip.h \
	dzipcon.h \
	dzip-zconf.h \
	dzip-zlib.h \
	encode.c \
	list.c \
	main.c \
	uncompress.c \
	v1code.c \
	zlib/adler32.c \
	zlib/deflate.c \
	zlib/deflate.h \
	zlib/infblock.c \
	zlib/infblock.h \
	zlib/infcodes.c \
	zlib/infcodes.h \
	zlib/inffast.c \
	zlib/inffast.h \
	zlib/inffixed.h \
	zlib/inflate.c \
	zlib/inftrees.c \
	zlib/inftrees.h \
	zlib/infutil.c \
	zlib/infutil.h \
	zlib/trees.c \
	zlib/trees.h \
	zlib/zconf.h \
	zlib/zlib.h \
	zlib/zutil.h

bin_PROGRAMS = dzip
dzip_SOURCES = conmain.c
dzip_LDADD = libdzip.la

