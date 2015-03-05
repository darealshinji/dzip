#!/bin/sh
rm -rf autom4te.cache build-aux m4 aclocal.m4
cp -r m4macros m4
autoreconf -ivf
