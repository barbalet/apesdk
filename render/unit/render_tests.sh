#!/bin/bash
# Build render unit tests from source.

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

rm -f ./*.o ./render_tests

gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../render -c ../../toolkit/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../render -c ../../render/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../render -c render_tests.c -o render_tests.o -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o render_tests ./*.o -lz -lm -lpthread -w

rm -f ./*.o
