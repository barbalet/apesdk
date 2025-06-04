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

gcc ${CFLAGS} ${COMMANDLINEE} -c ../*.c -lz -lm -lpthread -w


gcc ${CFLAGS} ${COMMANDLINEE} -c test_math.c -o test_math.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_math *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_math.o


gcc ${CFLAGS} ${COMMANDLINEE} -c test_prototype.c -o test_prototype.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_prototype *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_prototype.o



gcc ${CFLAGS} ${COMMANDLINEE} -c test_object_string.c -o test_object_string.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_object_string *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_object_string.o

gcc ${CFLAGS} ${COMMANDLINEE} -c test_object_file.c -o test_object_file.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_object_file *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_object_file.o


gcc ${CFLAGS} ${COMMANDLINEE} -c test_object_file_vect2.c -o test_object_file_vect2.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_object_file_vect2 *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_object_file_vect2.o



gcc ${CFLAGS} ${COMMANDLINEE} -c test_object.c -o test_object.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_object *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

rm test_object.o
