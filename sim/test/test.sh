#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2013 Tom Barbalet. All rights reserved.
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

SOURCEDIR=..

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

gcc ${CFLAGS} -c $SOURCEDIR/noble/io.c -o io.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/math.c -o math.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/parse.c -o parse.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/interpret.c -o interpret.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/land.c -o land.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/audio.c -o audio.o
gcc ${CFLAGS} -DEXECUTE_DEBUG -c $SOURCEDIR/noble/execute.c -o execute.o

gcc ${CFLAGS} -c test_execute.c -o test_execute.o

gcc ${CFLAGS} -I/usr/include -o test_execute *.o -lz -lm -lpthread

#./test_execute

rm test_execute

rm *.o
