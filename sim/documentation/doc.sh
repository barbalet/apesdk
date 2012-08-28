#	doc.sh
#
#	=============================================================
#
#   Copyright 1996-2012 Tom Barbalet. All rights reserved.
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
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
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

gcc ${CFLAGS} -c ../noble/io.c -o io.o
gcc ${CFLAGS} -c ../noble/math.c -o math.o
gcc ${CFLAGS} -c ../noble/parse.c -o parse.o
gcc ${CFLAGS} -c ../noble/interpret.c -o interpret.o
gcc ${CFLAGS} -c ../noble/land.c -o land.o

gcc ${CFLAGS} -c ../entity/being.c -o being.o
gcc ${CFLAGS} -c ../entity/body.c -o body.o
gcc ${CFLAGS} -c ../entity/metabolism.c -o metabolism.o
gcc ${CFLAGS} -c ../entity/brain.c -o brain.o
gcc ${CFLAGS} -c ../entity/social.c -o social.o
gcc ${CFLAGS} -c ../entity/episodic.c -o episodic.o
gcc ${CFLAGS} -c ../entity/food.c -o food.o
gcc ${CFLAGS} -c ../entity/drives.c -o drives.o

gcc ${CFLAGS} -c ../universe/file.c -o file.o
gcc ${CFLAGS} -c ../universe/console.c -o console.o
gcc ${CFLAGS} -c ../universe/sim.c -o sim.o

gcc ${CFLAGS} -c documentation.c -o documentation.o

gcc ${CFLAGS} -I/usr/include -o ./doc io.o math.o parse.o interpret.o console.o documentation.o being.o body.o social.o episodic.o food.o drives.o sim.o file.o brain.o metabolism.o land.o

rm *.o

./doc

rm doc



