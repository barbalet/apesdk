#!/bin/bash
#	linux_gui.sh
#
#	=============================================================
#
#   Copyright 1996-2012 Tom Barbalet. All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use,
#   copy, modify, merge, publish, distribute, sublicense, and/or
#   sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following
#   conditions:
#
#   The above copyright notice and this permission notice shall be
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.
#
#   This software and Noble Ape are a continuing work of Tom Barbalet,
#   begun on 13 June 1996. No apes or cats were harmed in the writing
#   of this software.
 
SOURCEDIR=.  
ARCH_TYPE=`uname -m`  
  
if [ ${ARCH_TYPE} == "x86_64" ]  
then  
	LIB=lib64  
else  
	LIB=lib  
fi  
 
if [ $# -ge 1 -a "$1" == "--debug" ]  
then  
	CFLAGS=-g  
else  
	CFLAGS=-O2  
fi  

OS=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | tr '[:upper:]' '[:lower:]')
if [ -z "$OS" ]; then    
    OS=$(awk '/ID=/' /etc/*-release | sed 's/ID=//' | tr '[:upper:]' '[:lower:]')
    set -- $OS
    OS=$1
fi
VERSION=$(awk '/DISTRIB_RELEASE=/' /etc/*-release | sed 's/DISTRIB_RELEASE=//' | sed 's/[.]0/./')
if [ -z "$VERSION" ]; then
    VERSION=$(awk '/VERSION_ID=/' /etc/*-release | sed 's/VERSION_ID=//' | sed 's/[.]0/./')
fi
# Convert floating point version into an integer
INT_VERSION=`echo $VERSION '*100' | bc -l | awk -F '.' '{ print $1; exit; }'`


if [[ $OS == "fedora" || $OS == "redhat" || $OS == "centos" ]]; then
 
	INCLUDES="-I/usr/include/openmotif -I/usr/include/gtk-2.0 -I/usr/${LIB}/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/include"  
 
	LIBS2="`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`-lm"

elif [[ $OS == "ubuntu" || $OS == "debian" || $OS == "solusos" ]]; then

	INCLUDES="-I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/include"

	LIBS2="`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` -lm"

fi

./linux.sh --additional

gcc ${CFLAGS} ${INCLUDES} -c $SOURCEDIR/gtk/platform.c -o platform.o ${LIBS2}

gcc ${CFLAGS} ${INCLUDES} -o $SOURCEDIR/../na io.o math.o parse.o interpret.o being.o brain.o metabolism.o land.o social.o episodic.o food.o drives.o sim.o file.o genealogy.o body.o draw.o control.o platform.o console.o speak.o ${LIBS2}

#gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/cle.c -o cleweb.o
#gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/web.c -o web.o
#gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/pnglite.c -o pnglite.o

#gcc -Wall -ansi -pedantic -O3 -o ../naweb cleweb.o pnglite.o -lz sim.o file.o genealogy.o body.o being.o metabolism.o brain.o console.o land.o social.o episodic.o food.o drives.o math.o io.o parse.o interpret.o graph.o web.o speak.o -lm -lpthread

rm *.o 
