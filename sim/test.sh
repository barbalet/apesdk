#	 test.sh
#
#	 =============================================================
#
#    Copyright 1996-2012 Tom Barbalet. All rights reserved.
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

cd ..

rm natest

cd sim

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

gcc ${CFLAGS} -c noble/io.c -o io.o
gcc ${CFLAGS} -c noble/math.c -o math.o
gcc ${CFLAGS} -c noble/parse.c -o parse.o
gcc ${CFLAGS} -c noble/interpret.c -o interpret.o

gcc ${CFLAGS} -c universe/being.c -o being.o
gcc ${CFLAGS} -c universe/body.c -o body.o
gcc ${CFLAGS} -c universe/brain.c -o brain.o
gcc ${CFLAGS} -c universe/land.c -o land.o
gcc ${CFLAGS} -c command/genealogy.c -o genealogy.o
gcc ${CFLAGS} -c command/graph.c -o graph.o
gcc ${CFLAGS} -c universe/social.c -o social.o
gcc ${CFLAGS} -c universe/file.c -o file.o
gcc ${CFLAGS} -c universe/sim.c -o sim.o

gcc ${CFLAGS} -c test/test.c -o test.o

gcc ${CFLAGS} -I/usr/include -o ../natest io.o math.o parse.o interpret.o being.o body.o brain.o land.o social.o genealogy.o file.o sim.o test.o graph.o

cd ..

./natest ./sim/test/test_1.txt
./natest ./sim/test/test_2.txt ./sim/test/test_1.txt
./natest ./sim/test/test_3.txt ./sim/test/test_1.txt

diff ./sim/test/test_2.txt ./sim/test/test_3.txt

cd sim

rm *.o

cd test

rm test_1.txt
rm test_2.txt
rm test_3.txt

cd ..


