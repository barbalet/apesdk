#	 lancelink.sh
#
#	 =============================================================
#
#    Copyright 1996-2013 Tom Barbalet. All rights reserved.
#
#    Permission is hereby granted, free of charge, to any person
#    obtaining a copy of this software and associated documentation
#    files (the "Software"), to deal in the Software without
#    restriction, including without limitation the rights to use,
#    copy, modify, merge, publish, distribute, sublicense, and/or
# 	 sell copies of the Software, and to permit persons to whom the
#    Software is furnished to do so, subject to the following
#    conditions:
#
#    The above copyright notice and this permission notice shall be
#	 included in all copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#    OTHER DEALINGS IN THE SOFTWARE.
#
#    This software and Noble Ape are a continuing work of Tom Barbalet,
#    begun on 13 June 1996. No apes or cats were harmed in the writing
#    of this software.

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c noble/io.c -o io.o
gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c noble/math.c -o math.o
gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c noble/parse.c -o parse.o
gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c noble/interpret.c -o interpret.o
gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c noble/skim.c -o skim.o

gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c lance/commands.c -o commands.o
gcc ${CFLAGS} -DCOMMAND_LINE_DEBUG -c lance/lance.c -o lance.o

gcc ${CFLAGS} -I/usr/include -o ../lance io.o math.o parse.o interpret.o lance.o commands.o skim.o

cd ..

./lance ./sim/lance/basic_check.txt

cd sim

rm *.o

