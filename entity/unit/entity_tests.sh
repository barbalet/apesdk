#!/bin/bash
# Build entity unit tests from source.

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

rm -f ./*.o ./entity_tests

gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../toolkit/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../script/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../render/graph.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../sim/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../entity/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../universe/*.c -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c entity_tests.c -o entity_tests.o -lz -lm -lpthread -w
gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o entity_tests ./*.o -lz -lm -lpthread -w

rm -f ./*.o
