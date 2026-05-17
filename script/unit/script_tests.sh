#!/bin/bash
# Build script unit tests from source.

set -euo pipefail

if [ $# -ge 1 ] && [ "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2
fi

if [ $# -ge 1 ] && [ "$1" == "--coverage" ]
then
    COMMANDLINEE="-ftest-coverage -fprofile-arcs"
else
    COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

rm -f ./*.o ./script_tests

gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -c ../../toolkit/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -c ../../script/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -c script_tests.c -o script_tests.o -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o script_tests ./*.o -lz -lm -lpthread -w

rm -f ./*.o
