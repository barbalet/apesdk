#!/bin/bash
#	linux.sh
#
#	=============================================================
#
#   Copyright 1996-2014 Tom Barbalet. All rights reserved.
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

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

if [ $# -ge 1 -a "$1" == "--additional" ]
then
COMMANDLINEE=-DNOTHING_NEEDED_HERE
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/io.c -o io.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/math.c -o math.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/parse.c -o parse.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/interpret.c -o interpret.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/land.c -o land.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/audio.c -o audio.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/execute.c -o execute.o

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/being.c -o being.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/body.c -o body.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/brain.c -o brain.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/social.c -o social.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/episodic.c -o episodic.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/food.c -o food.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/drives.c -o drives.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/entity/speak.c -o speak.o

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/universe/file.c -o file.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/universe/console.c -o console.o
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/universe/sim.c -o sim.o

if [ $# -ge 1 -a "$1" == "--test" ]
then

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/test.c -o test.o


gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o $SOURCEDIR/../../natest *.o -lz -lm -lpthread

rm *.o

cd ../../
./natest
cd nobleape/sim/

else

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/longterm.c -o longterm.o

if [ $# -ge 1 -a "$1" == "--additional" ]
then

gcc ${CFLAGS} -c $SOURCEDIR/gui/draw.c -o draw.o

else

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o $SOURCEDIR/../../nalongterm *.o -lz -lm -lpthread

rm *.o

fi

fi
