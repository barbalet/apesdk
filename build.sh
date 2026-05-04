#!/bin/sh
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

set -eu

usage()
{
    echo "usage: ./build.sh [--debug] [--additional] [--output PATH] [--prefix PATH]"
}

CC=${CC:-cc}
CFLAGS=${CFLAGS:-"-O2"}
CPPFLAGS=${CPPFLAGS:-}
LDFLAGS=${LDFLAGS:-}
LIBS=${LIBS:-"-lz -lm -pthread"}
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
OUTPUT=simape
PREFIX=

while [ $# -gt 0 ]
do
    case "$1" in
        --debug)
            CFLAGS="-g"
            ;;
        --additional)
            COMMANDLINEE=-DNOTHING_NEEDED_HERE
            ;;
        --output)
            shift
            if [ $# -eq 0 ]
            then
                usage
                exit 1
            fi
            OUTPUT=$1
            ;;
        --prefix)
            shift
            if [ $# -eq 0 ]
            then
                usage
                exit 1
            fi
            PREFIX=$1
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
    shift
done

BUILD_DIR=.build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
trap 'rm -rf "$BUILD_DIR"' EXIT HUP INT TERM

for file in ./toolkit/*.c ./script/*.c ./render/*.c ./sim/*.c ./entity/*.c ./universe/*.c ./longterm.c
do
    object="$BUILD_DIR/$(basename "$file" .c).o"
    "$CC" $CFLAGS $CPPFLAGS "$COMMANDLINEE" -I. -c "$file" -o "$object" -w
done

"$CC" $CFLAGS $LDFLAGS -o "$OUTPUT" "$BUILD_DIR"/*.o $LIBS

if [ -n "$PREFIX" ]
then
    mkdir -p "$PREFIX/bin"
    cp "$OUTPUT" "$PREFIX/bin/simape"
fi
