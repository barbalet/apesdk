/****************************************************************

 console.c

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

/*! \file   n_console.c
 *  \brief  Covers the low level input and output relating to console.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../toolkit/toolkit.h"

simulated_console_command *local_commands = 0L;

static n_int command_line_execution;
static n_int command_line_external_exit = 0;

void  io_command_line_execution_set( void )
{
    command_line_execution = 1;
}

n_int io_command_line_execution( void )
{
    return command_line_execution;
}

void io_entry_execution( n_int argc, n_string *argv )
{
    if ( argv )
    {
        if ( ( argc == 2 ) && ( argv[1][1] == 'c' ) )
        {
            io_command_line_execution_set();
        }
    }
}


void io_help_line( simulated_console_command *specific, n_console_output output_function )
{
    n_string_block  string_line = {0};
    io_three_string_combination( string_line, specific->command, specific->addition, specific->help_information, 28 );
    output_function( string_line );
}

n_int io_help( void *ptr, n_string response, n_console_output output_function )
{
    n_int loop = 0;
    n_int response_len = 0;
    n_int found = 0;

    if ( response != 0L )
    {
        response_len = io_length( response, 1024 );
    }

    if ( response_len == 0 )
    {
        output_function( "Commands:" );
    }

    do
    {
        if ( local_commands[loop].function != 0L )
        {
            if ( ( local_commands[loop].help_information ) && ( local_commands[loop].help_information[0] != 0 ) )
            {
                if ( response_len == 0 )
                {
                    io_help_line( &local_commands[loop], output_function );
                }
                else
                {
                    n_int command_len = io_length( local_commands[loop].command, 1024 );
                    n_int count = io_find( response, 0, response_len, local_commands[loop].command, command_len );
                    if ( count == command_len )
                    {
                        io_help_line( &local_commands[loop], output_function );
                        found = 1;
                    }
                }
            }
            loop++;
        }
    }
    while ( local_commands[loop].function != 0L );
    if ( ( response_len != 0 ) && ( found == 0 ) )
    {
        ( void )SHOW_ERROR( "Command not found, type help for more information" );
    }
    return 0;
}

n_int io_quit( void *ptr, n_string response, n_console_output output_function )
{
    return 1;
}

n_string io_console_entry_clean( n_string string, n_int length )
{
    return fgets( string, ( int )length, stdin );
}

n_string io_console_entry( n_string string, n_int length )
{
    return io_console_entry_clean( string, length );
}

void io_console_out( n_constant_string value )
{
    printf( "%s\n", value );
    fflush( stdout );
}

void io_console_quit( void )
{
    command_line_external_exit = 1;
}

n_int io_console( void *ptr, simulated_console_command *commands, n_console_input input_function, n_console_output output_function )
{
    n_string_block buffer;

    local_commands = commands;

    if ( ( input_function )( buffer, STRING_BLOCK_SIZE ) != 0L )
    {
        n_int  loop = 0;
        n_int buffer_len = io_length( buffer, STRING_BLOCK_SIZE );

        if ( ( commands[0].command == 0L ) && ( commands[0].function == 0L ) )
        {
            return SHOW_ERROR( "No commands provided" );
        }

        /* captures linux, mac and windows line ending issue */
        if ( IS_RETURN( buffer[buffer_len - 1] ) )
        {
            buffer[buffer_len - 1] = 0;
            buffer_len--;
        }
        if ( IS_RETURN( buffer[buffer_len - 1] ) )
        {
            buffer[buffer_len - 1] = 0;
            buffer_len--;
        }

        if ( buffer_len != 0 )
        {
            do
            {
                n_int command_len = io_length( commands[loop].command, 1024 );
                n_int count = io_find( ( n_string )buffer, 0, buffer_len, commands[loop].command, command_len );
                if ( count != -1 )
                {
                    n_int return_value;
                    n_console *function = commands[loop].function;
                    if ( IS_SPACE( buffer[count] ) )
                    {
                        return_value = ( *function )( ptr, ( n_string )&buffer[count + 1], output_function );
                        if ( command_line_external_exit )
                        {
                            return 1;
                        }
                        return return_value;
                    }
                    else if ( buffer[count] == 0 )
                    {
                        return_value = ( *function )( ptr, 0L, output_function );
                        if ( command_line_external_exit )
                        {
                            return 1;
                        }
                        return return_value;
                    }
                }
                loop++;
            }
            while ( ( commands[loop].command != 0L ) && ( commands[loop].function != 0L ) );

            ( void )SHOW_ERROR( "Command not found, type help for more information" );

            return 0;
        }
        else
        {
            return 0;
        }
    }
    return SHOW_ERROR( "Console failure" );
}

#if 0

#include <stdio.h>
#include <sys/termios.h>
#include <unistd.h>

int mygetch(void) {
    char ch;
    int error;
    static struct termios Otty, Ntty;

    fflush(stdout);
    tcgetattr(0, &Otty);
    Ntty = Otty;

    Ntty.c_iflag  =  0;     /* input mode       */
    Ntty.c_oflag  =  0;     /* output mode      */
    Ntty.c_lflag &= ~ICANON;    /* line settings    */

#if 1
    /* disable echoing the char as it is typed */
    Ntty.c_lflag &= ~ECHO;  /* disable echo     */
#else
    /* enable echoing the char as it is typed */
    Ntty.c_lflag |=  ECHO;  /* enable echo      */
#endif

    Ntty.c_cc[VMIN]  = CMIN;    /* minimum chars to wait for */
    Ntty.c_cc[VTIME] = CTIME;   /* minimum wait time    */

#if 1
    /*
    * use this to flush the input buffer before blocking for new input
    */
    #define FLAG TCSAFLUSH
#else
    /*
    * use this to return a char from the current input buffer, or block if
    * no input is waiting.
    */
    #define FLAG TCSANOW

#endif

    if ((error = tcsetattr(0, FLAG, &Ntty)) == 0) {
        error  = read(0, &ch, 1 );        /* get char from stdin */
        error += tcsetattr(0, FLAG, &Otty);   /* restore old settings */
    }

    return (error == 1 ? (int) ch : -1 );
}

#endif

