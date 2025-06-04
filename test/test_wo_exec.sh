#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2024 Tom Barbalet. All rights reserved.
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
#   This software is a continuing work of Tom Barbalet, begun on
#   13 June 1996. No apes or cats were harmed in the writing of
#   this software.

SOURCEDIR=..

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

if [ $# -ge 1 -a "$1" == "--coverage" ]
then
COMMANDLINEE="-ftest-coverage -fprofile-arcs"
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

gcc ${CFLAGS} ${COMMANDLINEE} -c ../toolkit/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -c ../script/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -c ../render/*.c -lz -lm -lpthread -w



gcc  ${CFLAGS} ${COMMANDLINEE} -c ../sim/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ../entity/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ../universe/*.c -lz -lm -lpthread -w

gcc ${CFLAGS} ${COMMANDLINEE} -c test_sim.c -o test_sim.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi


gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_sim *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_sim.o

gcc ${CFLAGS} ${COMMANDLINEE} -c test_sim_time.c -o test_sim_time.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_sim_time *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_sim_time.o
