#!/bin/bash
# Build entity unit tests from source.

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

rm -f ./*.o ./entity_tests ./entity_tests.tmp

${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../toolkit/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../script/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../render/graph.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../sim/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../entity/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c ../../universe/*.c -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I../../toolkit -I../../script -I../../render -I../../sim -I../../entity -I../../universe -c entity_tests.c -o entity_tests.o -lz -lm -lpthread
${CC} ${CFLAGS} ${COMMANDLINEE} ${TEST_WARNINGS} -I/usr/include -o entity_tests.tmp ./*.o -lz -lm -lpthread
mv entity_tests.tmp entity_tests

rm -f ./*.o
