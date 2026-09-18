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


set -euo pipefail

SOURCEDIR=..

if [ "${1:-}" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

: "${CC:=gcc}"
: "${TEST_WARNINGS:=-Wall -Wextra}"
: "${WERROR:=0}"
if [ "$WERROR" = 1 ]; then TEST_WARNINGS="$TEST_WARNINGS -Werror"; fi
rm -f ./*.o ./test_immune ./test_immune.tmp

if { [ $# -ge 1 ] && [ "$1" == "--coverage" ]; } || [ "${COVERAGE:-0}" = 1 ]
then
COMMANDLINEE="-ftest-coverage -fprofile-arcs"
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../toolkit/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../script/*.c -lz -lm -lpthread

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../render/graph.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../sim/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../entity/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c ../../universe/*.c -lz -lm -lpthread

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -c test_immune.c -o test_immune.o -lz -lm -lpthread

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I/usr/include -o test_immune.tmp *.o -lz -lm -lpthread
mv test_immune.tmp test_immune

rm *.o
