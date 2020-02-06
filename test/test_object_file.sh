#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2020 Tom Barbalet. All rights reserved.
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
#   This software is a continuing work of Tom Barbalet, begun on
#   13 June 1996. No apes or cats were harmed in the writing of
#   this software.

SOURCEDIR=..

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

gcc  ${CFLAGS} -c ../toolkit/*.c -lz -lm -lpthread -w

gcc ${CFLAGS} -c test_object_file.c -o test_object_file.o
gcc ${CFLAGS} -I/usr/include -o test_object_file *.o -lz -lm -lpthread


./test_object_file example0c.json
diff example0c.json 2xample0c.json

./test_object_file example1.json
diff example1.json 2xample1.json

./test_object_file example1b.json
diff example1b.json 2xample1b.json

./test_object_file example1c.json
diff example1c.json 2xample1c.json

./test_object_file example2.json
diff example2.json 2xample2.json

./test_object_file example2c.json
diff example2c.json 2xample2c.json

./test_object_file example3.json
diff example3.json 2xample3.json

./test_object_file example3c.json
diff example3c.json 2xample3c.json

./test_object_file example3d.json
diff example3d.json 2xample3d.json

./test_object_file example3e.json
diff example3e.json 2xample3e.json

./test_object_file example4.json
diff example4.json 2xample4.json

./test_object_file example4b.json
diff example4b.json 2xample4b.json

./test_object_file example5.json
diff example5.json 2xample5.json

./test_object_file battle.json
diff battle.json 2attle.json

./test_object_file sim.json
diff sim.json 2im.json

./test_object_file example6.json
diff example6.json 2xample6.json

rm 2*.json

rm test_object_file

rm *.o
