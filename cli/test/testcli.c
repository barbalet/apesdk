/****************************************************************

    testcli.c

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

#include "../../toolkit/toolkit.h"
#include "../cli.h"

#include <stdio.h>


n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

static simulated_console_command test_commands[] =
{
    {&io_help,               "help",           "[(command)]",          "Displays a list of all the commands"},

    {&io_quit,           "quit",           "",                     "Quits the console"},
    {&io_quit,           "exit",           "",                     ""},
    {&io_quit,           "close",          "",                     ""},


    {0L, 0L},
};

static n_int entry_num = 0;

static n_string execution[3] = {"help", "help quit", "quit"};

n_string autoentry( n_string string, n_int length )
{
    entry_num++;
    printf("> %s\n",execution[entry_num - 1]);
    sprintf(string, "%s\n", execution[entry_num - 1]);
    return string;
}

void autoout( n_constant_string value )
{
    printf( "%s\n", value );
    fflush( stdout );

}

int main(int argc, const char * argv[]) {
    do
    {}
    while ( io_console( 0L,
                        (simulated_console_command *)test_commands,
                       autoentry,/*io_console_entry,*/
                       autoout ) == 0 );
    return 0;
}
