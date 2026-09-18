#!/bin/bash
#	build.sh
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

cd "$(dirname "$0")"

set -euo pipefail

if [ "${1:-}" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

: "${CC:=gcc}"
: "${BUILD_WARNINGS:=-Wall -Wextra}"
rm -f ./*.o ./../simape ./../simape.tmp

if [ "${1:-}" == "--additional" ]
then
COMMANDLINEE=-DNOTHING_NEEDED_HERE
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./toolkit/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./script/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./render/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./sim/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./entity/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./universe/*.c -lz -lm -lpthread

${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -c ./longterm.c -o longterm.o

${CC} ${CFLAGS} ${COMMANDLINEE} ${BUILD_WARNINGS} -I/usr/include -o ./../simape.tmp *.o -lz -lm -lpthread
mv ./../simape.tmp ./../simape

rm *.o
