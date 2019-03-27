#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2019 Tom Barbalet. All rights reserved.
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

if [ $# -ge 1 -a "$1" == "--additional" ]
then
COMMANDLINEE=-DNOTHING_NEEDED_HERE
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/console.c -o console.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/file.c -o file.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/memory.c -o memory.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/io.c -o io.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/math.c -o math.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/parse.c -o parse.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/interpret.c -o interpret.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/land.c -o land.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/object.c -o object.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/audio.c -o audio.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/tile.c -o tile.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/noble/territory.c -o territory.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -c test_math.c -o test_math.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -I/usr/include -o test_math *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

./test_math

rm test_math
rm test_math.o

gcc ${CFLAGS} -c test_object.c -o test_object.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -I/usr/include -o test_object *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

./test_object

rm test_object
rm test_object.o

gcc ${CFLAGS} -c test_io.c -o test_io.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -I/usr/include -o test_io *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

./test_io

diff check_file.txt compare_file.txt

rm test_io.o
rm test_io
rm compare_file.txt

gcc ${CFLAGS} -c test_apescript.c -o test_apescript.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -I/usr/include -o test_apescript *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

./test_apescript check_apescript.txt

rm test_apescript
rm test_apescript.o

gcc ${CFLAGS} -c $SOURCEDIR/entity/being.c -o being.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/entity/body.c -o body.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/entity/brain.c -o brain.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -c $SOURCEDIR/entity/drives.c -o drives.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/entity/social.c -o social.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/entity/speak.c -o speak.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -c $SOURCEDIR/entity/episodic.c -o episodic.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/entity/food.c -o food.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -c $SOURCEDIR/universe/loop.c -o loop.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/universe/command.c -o command.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/universe/transfer.c -o transfer.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi
gcc ${CFLAGS} -c $SOURCEDIR/universe/sim.c -o sim.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -c test_sim.c -o test_sim.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} -I/usr/include -o test_sim *.o -lz -lm -lpthread -w
if [ $? -ne 0 ]
then
exit 1
fi

./test_sim

./test_sim

rm test_sim

rm *.o
