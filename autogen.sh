#!/bin/sh
rm -rf autom4te.cache build-aux m4 aclocal.m4
mkdir -p m4
autoreconf -ivf
