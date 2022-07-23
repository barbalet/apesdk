/****************************************************************

    cli.h

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

#include "../toolkit/toolkit.h"

#ifndef APESDK_CLI_H
#define APESDK_CLI_H

typedef n_string ( n_console_input )( n_string value, n_int length );

typedef void ( n_console_output )( n_constant_string value );

typedef void (sequential_commands)(void * ptr, n_console_output output);

typedef n_int (seqence_offering)(void * ptr);


n_int useful_executing( void );


typedef n_int ( n_console )( void *ptr, n_string response, n_console_output output_function );

typedef struct
{
    n_console *function;
    n_string    command;
    n_string    addition;
    n_string    help_information;
} simulated_console_command;

n_int useful_next( void *ptr, n_string response, n_console_output output_function );
n_int useful_previous( void *ptr, n_string response, n_console_output output_function );

n_int useful_run( void *ptr, n_string response, n_console_output output_function );
n_int useful_interval( void *ptr, n_string response, n_console_output output_function );
n_int useful_stop( void *ptr, n_string response, n_console_output output_function );

n_int useful_quit( void *ptr, n_string response, n_console_output output_function );

n_int useful_simulation( void *ptr, n_string response, n_console_output output_function );
n_int useful_step( void *ptr, n_string response, n_console_output output_function );


void cli_set_simulation(n_console * local_lifter);
n_int cli_execute_simulation( void *ptr, n_string response, n_console_output output_function );

void cli_set_cycle_entity(seqence_offering * cycle, sequential_commands * entity);

n_int cli_execute_cycle(void * ptr);
void cli_execute_entity(void * ptr, n_console_output output_function );


void cli_file_interaction_set(n_int num);
n_int cli_file_interaction(void);

void cli_set_previous_next(sequential_commands * command_previous, sequential_commands * command_next);
void cli_execute_previous(void * ptr, n_console_output output);
void cli_execute_next(void * ptr, n_console_output output);

void       io_entry_execution( n_int argc, n_string *argv );
void       io_command_line_execution_set( void );
n_int      io_command_line_execution( void );

n_int      io_quit( void *ptr, n_string response, n_console_output output_function );
n_int      io_help( void *ptr, n_string response, n_console_output output_function );
n_string   io_console_entry_clean( n_string string, n_int length );
n_string   io_console_entry( n_string string, n_int length );
void       io_console_out( n_constant_string value );
n_int      io_console( void *ptr, simulated_console_command *commands, n_console_input input_function, n_console_output output_function );

void       io_help_line(simulated_console_command *specific, n_console_output output_function );

void       io_console_quit( void );

#endif
