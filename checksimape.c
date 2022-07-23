/****************************************************************

    checksimape.c

    =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    This software is a continuing work of Tom Barbalet, begun on
    13 June 1996. No apes or cats were harmed in the writing of
    this software.

****************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>


#include "toolkit/toolkit.h"
#include "sim/sim.h"
#include "script/script.h"
#include "universe/universe.h"


n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

static n_int entry_num = 0;

n_string autoentry( n_string string, n_int length )
{
    n_string execution[20] = {"help", "event on", "run 1 min", "sim", "ape", "next", "ape", "next", "ape", "next", "ape", "previous", "ape", "memory", "sim", "run 1 week", "graph", "run 1 week", "graph", "quit"};
    
    entry_num++;
    printf("> %s\n",execution[entry_num - 1]);
    sprintf(string, "%s\n", execution[entry_num - 1]);
    return string;
}

int command_line_run( void )
{
    srand( ( unsigned int ) time( NULL ) );
    sim_console_debug( autoentry, rand() );
    
    return 1;
}

int main( int argc, n_string argv[] )
{
    return command_line_run();
}

