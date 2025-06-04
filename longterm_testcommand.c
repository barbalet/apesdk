/****************************************************************

    longterm.c

    =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#define _CRT_SECURE_NO_WARNINGS

#define CONSOLE_ONLY /* Please maintain this define until after ALIFE XIII July 22nd */
#define CONSOLE_REQUIRED
#undef  AUDIT_FILE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#endif

#include "toolkit/toolkit.h"
#include "sim/sim.h"
#include "script/script.h"
#include "universe/universe.h"


n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

static n_int quidjibo_chapter = 0;

static n_string values[]= {"step 200", "stop", "step 200", "stop", "step 20", "list", "stop", "help", "step 250", "stop", "step 2", "stop", "step 1",  "list", "sim", "stop", "quit", "quit", "", ""};

n_string autoentry( n_string string, n_int length )
{

    quidjibo_chapter++;
    
    usleep(2000000);
    printf("%ld > %s\n",quidjibo_chapter, values[quidjibo_chapter-1]);
    sprintf(string, "%s\n", values[quidjibo_chapter-1]);
    
    return string;
}

int command_line_run( void )
{

    srand( ( unsigned int ) time( NULL ) );
    sim_console( 0L, rand() );

    return ( 0 );
}


int main( int argc, n_string argv[] )
{
    sim_set_console_input(autoentry);
    return command_line_run();
}

