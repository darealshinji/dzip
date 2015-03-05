AM_CFLAGS   = -m32 -Wall -Wno-unused-result
AM_CPPFLAGS = -I./include -DDZ_PREFIX
AM_LDFLAGS  = -m32

include_HEADERS = \
	include/dzip.h \
	include/dzipcon.h \
	include/dzip-zconf.h \
	include/dzip-zlib.h

lib_LTLIBRARIES = libdzip.la
libdzip_la_SOURCES = \
	compress.c \
	uncompress.c \
	list.c \
	crc32.c \
	encode.c \
	decode.c \
	v1code.c \
	main.c \
	conmain-lib.c \
	delete.c \
	zlib/adler32.c \
	zlib/deflate.c \
	zlib/trees.c \
	zlib/inflate.c \
	zlib/infblock.c \
	zlib/inftrees.c \
	zlib/infcodes.c \
	zlib/infutil.c \
	zlib/inffast.c

bin_PROGRAMS = dzip
dzip_SOURCES = conmain.c
dzip_LDADD = libdzip.la

