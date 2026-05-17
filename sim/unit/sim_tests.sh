#!/bin/bash
# Build sim unit tests from source.

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

rm -f ./*.o ./sim_tests

gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../sim -c ../../toolkit/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../sim -c ../../sim/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../sim -c sim_tests.c -o sim_tests.o -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o sim_tests ./*.o -lz -lm -lpthread -w

rm -f ./*.o
