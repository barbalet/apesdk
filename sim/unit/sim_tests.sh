#!/bin/bash
# Build sim unit tests from source.

set -euo pipefail

if [ $# -ge 1 ] && [ "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2
fi

: "${CC:=gcc}"
: "${TEST_WARNINGS:=-Wall -Wextra}"
: "${WERROR:=0}"
if [ "$WERROR" = 1 ]; then TEST_WARNINGS="$TEST_WARNINGS -Werror"; fi

if { [ $# -ge 1 ] && [ "$1" == "--coverage" ]; } || [ "${COVERAGE:-0}" = 1 ]
then
    COMMANDLINEE="-ftest-coverage -fprofile-arcs"
else
    COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

rm -f ./*.o ./sim_tests ./sim_tests.tmp

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../sim -c ../../toolkit/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../sim -c ../../sim/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../sim -c sim_tests.c -o sim_tests.o -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I/usr/include -o sim_tests.tmp ./*.o -lz -lm -lpthread
mv sim_tests.tmp sim_tests

rm -f ./*.o
