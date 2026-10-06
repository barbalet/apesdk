#!/bin/bash
# Build script unit tests from source.

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

rm -f ./*.o ./script_tests ./script_tests.tmp

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -c ../../toolkit/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -c ../../script/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -c script_tests.c -o script_tests.o -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I/usr/include -o script_tests.tmp ./*.o -lz -lm -lpthread
mv script_tests.tmp script_tests

rm -f ./*.o
