#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2016 Tom Barbalet. All rights reserved.
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

gcc ${CFLAGS} -c $SOURCEDIR/noble/audio.c -o audio.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/compress.c -o compress.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/execute.c -o execute.o

gcc ${CFLAGS} -c $SOURCEDIR/noble/interpret.c -o interpret.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/io.c -o io.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/land.c -o land.o

gcc ${CFLAGS} -c $SOURCEDIR/noble/math.c -o math.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/network.c -o network.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/parse.c -o parse.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/object.c -o object.o

gcc ${CFLAGS} -c test_math.c -o test_math.o

gcc ${CFLAGS} -I/usr/include -o test_math *.o -lz -lm -lpthread

./test_math

rm test_math
rm test_math.o

gcc ${CFLAGS} -c test_object.c -o test_object.o
gcc ${CFLAGS} -I/usr/include -o test_object *.o -lz -lm -lpthread

./test_object

rm test_object
rm test_object.o

gcc ${CFLAGS} -c test_io.c -o test_io.o
gcc ${CFLAGS} -I/usr/include -o test_io *.o -lz -lm -lpthread

./test_io

diff check_file.txt compare_file.txt

rm test_io.o
rm test_io
rm compare_file.txt

gcc ${CFLAGS} -c test_apescript.c -o test_apescript.o

gcc ${CFLAGS} -I/usr/include -o test_apescript *.o

./test_apescript check_apescript.txt

rm test_apescript
rm test_apescript.o

gcc ${CFLAGS} -c $SOURCEDIR/entity/being.c -o being.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/body.c -o body.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/brain.c -o brain.o

gcc ${CFLAGS} -c $SOURCEDIR/entity/drives.c -o drives.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/social.c -o social.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/speak.c -o speak.o

gcc ${CFLAGS} -c $SOURCEDIR/entity/episodic.c -o episodic.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/food.c -o food.o

gcc ${CFLAGS} -c $SOURCEDIR/universe/console.c -o console.o
gcc ${CFLAGS} -c $SOURCEDIR/universe/file.c -o file.o
gcc ${CFLAGS} -c $SOURCEDIR/universe/sim.c -o sim.o

gcc ${CFLAGS} -c test_sim.c -o test_sim.o

gcc ${CFLAGS} -I/usr/include -o test_sim *.o

./test_sim

./test_sim

rm test_sim

rm *.o
