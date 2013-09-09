/****************************************************************

 lance.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32

#include "../noble/noble.h"

#else

#include "..\noble\noble.h"

#endif

#include <unistd.h>
#include <stdio.h>
#include "commands.h"

static n_interpret *interpret = 0L;

n_int draw_error(n_constant_string error_text)
{
    printf("ERROR: %s\n",error_text);
    return -1;
}


static n_int	lance_interpret(n_byte * buff, n_uint len)
{
    n_file	local;

    local . size = len;
    local . location = 0;
    local . data = buff;

    interpret = parse_convert(&local, VARIABLE_MAIN, (variable_string *)variable_codes);

    if(interpret == 0L)
    {
        return -1;
    }
    else
    {
        SC_DEBUG_ON; /* turn on debugging after script loading */
    }

    interpret->sc_input  = &commands_input;
    interpret->sc_output = &commands_output;

    interpret->input_greater   = VARIABLE_READWRITE;

    interpret->location = 0;
    interpret->leave = 0;
    interpret->localized_leave = 0;

    interpret->specific = 0;

    return 0;
}

static n_int lance_init(n_string interpret_string)
{
    n_file * source_file = io_file_new();

    if (io_disk_check(interpret_string) == 0)
    {
        return -1;
    }
    if (io_disk_read(source_file, interpret_string) == FILE_ERROR)
    {
        io_file_free(source_file);
        return -1;
    }
    if (lance_interpret(source_file->data,source_file->location) == -1)
    {
        io_file_free(source_file);
        return -1;
    }
    io_file_free(source_file);
    return 0;
}

static void lance_close()
{
#ifdef SKIM_TO_BRIANCODE
    skim_show(interpret->binary_code);
#endif
    interpret_cleanup(interpret);
}

#undef EXPLICIT_LOAD

int main(int argc, char *argv[])
{
    n_int   return_value = 0;

#ifdef EXPLICIT_LOAD

    if (lance_init("basic_check.txt") == -1)
    {
        printf("ERROR: Load Script %s failed\n",argv[1]);
        return 0;
    }

#else

    if (argc != 2)
    {
        printf("ERROR: Single script file string expected\n");
        return 0;
    }

    if (lance_init(argv[1]) == -1)
    {
        printf("ERROR: Load Script %s failed\n",argv[1]);
        return 0;
    }
#endif

    do
    {
        return_value = interpret_cycle(interpret, VARIABLE_EXIT - VARIABLE_FIRST_REAL_ONE, 0L,0,0L,0L);
        if (interpret->leave != 0)
        {
#ifdef COMMAND_LINE_DEBUG
            printf("...");
#endif
        }
    }
    while (return_value == 1);

    if (return_value == -1)
    {
        printf("ERROR: Script %s Ended with Error\n",argv[1]);
    }

    lance_close();

    return 1;
}
