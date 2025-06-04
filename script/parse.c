/****************************************************************

 parse.c

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

/*! \file   parse.c
 *  \brief  This parses ApeScript and produces the ApeScript byte-code.
 */

#include "../toolkit/toolkit.h"

#include "script.h"

#if defined(ROUGH_CODE_OUT) || defined(COMMAND_LINE_DEBUG)

#include <stdio.h>

#endif

#define	SYNTAX_NUM				19
#define	SYNTAX_WIDTH			4
static const n_byte	syntax_codes[SYNTAX_NUM][SYNTAX_WIDTH] =
{
    "-",
    "+",
    "*",
    "&",
    "^",
    "|",

    ">",
    "<",
    "==",
    "!=",
    "&&",
    "||",
    "/",

    "%",

    ">>",
    "<<",

    "<=",
    ">=",

    "="
};

static n_int	variable_num;
static n_int	number_num;

static n_int    quote_up;

#ifdef SCRIPT_DEBUG

static n_int	          tab_step = 0;
static variable_string	 *local_var_codes;

n_file                   *file_debug = 0L;
static n_int              single_entry = 1;

static void              *writable_selection;

n_file *scdebug_file_ready( void )
{
    return io_file_ready( single_entry, file_debug );
}

void scdebug_file_cleanup( void )
{
    io_file_cleanup( &single_entry, &file_debug );
}

void scdebug_writeon( void *ptr )
{
    writable_selection = ptr;
    io_file_writeon( &single_entry, &file_debug, 1 );
}

void scdebug_writeoff( void *ptr )
{
    if ( ptr != writable_selection )
    {
        return;
    }

    io_file_writeoff( &single_entry, file_debug );
}

void scdebug_string( void *ptr, n_constant_string string )
{
    if ( ptr != writable_selection )
    {
        return;
    }

    io_file_string( single_entry, file_debug, string );
}

n_string scdebug_variable( n_int variable )
{
    n_string return_value = 0L;
    if ( ( variable < VARIABLE_MAX )
#ifndef COMMAND_LINE_DEBUG
            && ( file_debug  != 0L )
#endif
       )
    {
        return_value = ( n_string ) local_var_codes[variable];
    }
    return return_value;
}

void scdebug_int( void *ptr, n_int number )
{
    if ( single_entry == 0 )
    {
        return;
    }

    if ( ptr != writable_selection )
    {
        return;
    }

#ifndef COMMAND_LINE_DEBUG
    if ( file_debug  != 0L )
    {
        io_writenumber( file_debug, number, 1, 0 );
    }
#else
    printf( "%d", ( int )number );
#endif
}

void scdebug_newline( void *ptr )
{
    if ( single_entry == 0 )
    {
        return;
    }

    if ( ptr != writable_selection )
    {
        return;
    }

#ifndef COMMAND_LINE_DEBUG
    if ( file_debug != 0L )
#endif
    {
        n_int loop = 0;
#ifndef COMMAND_LINE_DEBUG
        io_write( file_debug, "", 1 );
#else
        printf( "\n" );
#endif
        if ( tab_step > 0 )
        {
            while ( loop < tab_step )
            {
#ifndef COMMAND_LINE_DEBUG
                io_write( file_debug, "  ", 0 );
#else
                printf( "  " );
#endif
                loop++;
            }
        }
    }
}

void scdebug_tabstep( void *ptr, n_int steps )
{
    if ( ptr != writable_selection )
    {
        return;
    }

#ifndef COMMAND_LINE_DEBUG
    if ( file_debug != 0L )
#endif
    {
        tab_step += steps;
    }
}
#endif


static n_int parse_number_add( n_interpret *interpret, n_int out_value )
{
    n_int 	loop = 0;

    /* is this number already stored? */
    while ( loop < number_num )
    {
        if ( interpret->number_buffer[loop] == out_value )
        {
            return loop;
        }
        loop++;
    }
    /* if not, add it to the number store */
    interpret->number_buffer[loop] = out_value;
    if ( number_num < NUMBER_MAX )
    {
        number_num++;
    }
    else
    {
        return APESCRIPT_ERROR( 0L, AE_MAXIMUM_NUMBERS_REACHED );
    }
    return loop;
}

/* outputs the number of bytes to advance in the interpret stream */
static n_int parse_number( n_interpret *interpret, const n_byte *number )
{
    n_int 	out_value = 0;
    n_int 	point_counter = 0;

    /* read out the number from the interpret stream */
    do
    {
        n_byte temp = number[point_counter++];
        if ( ( !ASCII_NUMBER( temp ) ) && ( temp != 0 ) )
        {
            return APESCRIPT_ERROR( 0L, AE_NUMBER_EXPECTED ); /* this error should never occur */
        }
        out_value = ( out_value * 10 ) + ( temp - '0' );
    }
    while ( ( number[point_counter] != 0 ) && ( out_value > -1 ) );

    if ( ( out_value < 0 ) || ( out_value > 0x7fffffff ) )
    {
        return APESCRIPT_ERROR( 0L, AE_NUMBER_OUT_OF_RANGE );
    }

    return parse_number_add( interpret, out_value );
}

static n_int parse_quoted_string( n_interpret *interpret, n_constant_string string )
{
    return parse_number_add( interpret, ( n_int )math_hash_fnv1( string ) );
}

static n_byte parse_character( n_byte temp )
{
    if ( ASCII_QUOTE( temp ) )
    {
        quote_up ^= 1;
        return APESCRIPT_STRING;
    }

    if ( quote_up )
    {
        return APESCRIPT_STRING;
    }
    if ( ASCII_BRACES( temp ) || ASCII_BRACKET( temp ) )
    {
        return temp;
    }
    if ( ( ASCII_EQUAL( temp ) || ASCII_LOGICAL( temp ) ) || ( ASCII_ARITHMETIC( temp ) || ASCII_DIRECTIONAL( temp ) ) )
    {
        return APESCRIPT_OPERATOR;
    }
    if ( ASCII_NUMBER( temp ) )
    {
        return APESCRIPT_NUMBER;
    }
    if ( ASCII_TEXT( temp ) )
    {
        return APESCRIPT_TEXT;
    }
    if ( ASCII_SEMICOLON( temp ) )
    {
        return APESCRIPT_SEMICOLON;
    }
    return APESCRIPT_FAILURE;
}

static n_int parse_write_code( n_interpret *final_prog, n_byte value, n_byte code )
{
#ifdef ROUGH_CODE_OUT
    printf( "%c ", value );
#endif

    if ( io_file_write( final_prog->binary_code, value ) == -1 )
    {
        return APESCRIPT_ERROR( 0L, AE_MAXIMUM_SCRIPT_SIZE_REACHED );
    }
    if ( CODE_VALUE_REQUIRED( value ) )
    {
        if ( io_file_write( final_prog->binary_code, code ) == -1 )
        {
            return APESCRIPT_ERROR( 0L, AE_MAXIMUM_SCRIPT_SIZE_REACHED );
        }

#ifdef ROUGH_CODE_OUT
        printf( "%d ", code );
#endif
    }

#ifdef ROUGH_CODE_OUT
    if ( value == APESCRIPT_SEMICOLON || value == APESCRIPT_OPEN_BRACE || value == APESCRIPT_CLOSE_BRACE )
    {
        printf( "\n" );
    }
#endif
    return 0;
}

static n_int parse_string( const n_byte *test, const n_byte *compare, n_int number )
{
    n_int		loop = 0;
    while ( loop < number )
    {
        if ( test[loop] != compare[loop] )
        {
            return -1;
        }
        loop++;
    }
    return 1;
}

static n_int parse_buffer( n_interpret *final_prog, n_byte previous, const n_byte *buffer )
{
    variable_string *variable_codes = final_prog->variable_strings;
    n_int			 result = -1;
    n_int			 loop = 0;
    switch ( previous )
    {
    case ( APESCRIPT_NUMBER ):
        result = parse_number( final_prog, buffer ); /* this loads the number into the number buffer */
        if ( result == -1 )
        {
            return -1;
        }
        if ( parse_write_code( final_prog, previous, ( n_byte )result ) == -1 ) /* this writes the number allocation code */
        {
            return -1;
        }
        break;

    case ( APESCRIPT_STRING ):
        result = parse_quoted_string( final_prog, ( n_constant_string )buffer ); /* this loads the number into the number buffer */
        if ( result == -1 )
        {
            return -1;
        }
        if ( parse_write_code( final_prog, APESCRIPT_NUMBER, ( n_byte )result ) == -1 ) /* this writes the number allocation code */
        {
            return -1;
        }
        break;
    case ( APESCRIPT_TEXT ):
        while ( ( loop < variable_num ) && ( result == -1 ) )
        {
            if ( parse_string( variable_codes[loop], buffer, VARIABLE_WIDTH ) == 1 )
            {
                result = loop;
            }
            loop++;
        }
        if ( result == -1 )
        {
            if ( variable_num < VARIABLE_MAX )
            {
                n_int loop2 = 0;
                while ( loop2 < ( VARIABLE_WIDTH ) )
                {
                    variable_codes[variable_num][loop2] = buffer[loop2];
                    loop2++;
                }
                variable_num++;
            }
            else
            {
                return APESCRIPT_ERROR( 0L, AE_MAXIMUM_VARIABLES_REACHED );
            }
            result = loop;
        }
        if ( parse_write_code( final_prog, previous, ( n_byte )result ) == -1 )
        {
            return -1;
        }
        break;
    case ( APESCRIPT_OPERATOR ):
        while ( ( loop < SYNTAX_NUM ) && ( result == -1 ) )
        {
            if ( parse_string( syntax_codes[loop], buffer, SYNTAX_WIDTH ) == 1 )
            {
                result = loop;
            }
            loop++;
        }
        if ( result == -1 ) /* no error reported up until now */
        {
            return APESCRIPT_ERROR( 0L, AE_UNKNOWN_SYNTAX_PARSER_BUFFER );
        }
        if ( parse_write_code( final_prog, previous, ( n_byte )result ) == -1 )
        {
            return -1;
        }
        break;
    default:
    {
        n_byte	value;
        while ( ( value = buffer[loop++] ) != 0 )
        {
            if ( parse_write_code( final_prog, value, 0 ) == -1 )
            {
                return -1;
            }
        }
    }
    break;
    }
    return 0;
}

/**
 Turns an input file into an interpret-able pointer.
 @param input The file pointer containing the ApeScript text data.
 @param main_entry The variable defined as main. In the case of this implementation
 of ApeScript, being.
 @param variables The pointer to the variable string used for debugging to output the
 actual variable names.
 @return The interpreter pointer created from the file pointer.
 */
n_interpret 	*parse_convert( n_file *input, n_int main_entry, variable_string *variables )
{
    n_interpret *final_prog = 0L;
    n_byte	     *local_data;
    n_uint	      end_loop;
    n_uint	      loop = 0;
    n_int	     *local_number;
    n_byte	      buffer[ VARIABLE_WIDTH ];
    n_int	      buffer_size = 0;
    n_byte	      previous = 0;

    /* remove the white space from the input file */

    io_whitespace( input );

    /* perform the initial allocations */

    if ( ( final_prog = memory_new( sizeof( n_interpret ) ) ) == 0L )
    {
        return 0L;
    }

    if ( ( final_prog->binary_code = io_file_new() ) == 0L )
    {
        memory_free( ( void ** )&final_prog );
        return 0L;
    }

    if ( final_prog->binary_code->data == 0L )
    {
        interpret_cleanup( &final_prog );
        return 0L;
    }

    /* allow for the space for the size to be written after everything else has been written/calculated */

    final_prog->binary_code->location = SIZEOF_NUMBER_WRITE;

    final_prog->variable_strings = variables;
    final_prog->special_less    = ( VARIABLE_IF + 1 );
    final_prog->main_entry      = main_entry;

    local_number = final_prog->number_buffer;

    variable_num = main_entry + 1;

    number_num = 1;

    quote_up = 0;

    /* clear everything initially */

    while ( loop < NUMBER_MAX )
    {
        local_number[ loop++ ] = 0;
    }
    loop = 0;
    memory_erase( buffer, VARIABLE_WIDTH );

    local_data = input->data;
    end_loop = input->size;

    /* work through each character in the input file */

    while ( loop < end_loop )
    {
        n_byte	temp = local_data[ loop++ ];
        /* each character has a particular type */
        n_byte	convert = parse_character( temp );
        /* if it is a failure type i.e. not to be used, then fail */
        if ( convert == APESCRIPT_FAILURE )
        {
            interpret_cleanup( &final_prog );
            ( void )APESCRIPT_ERROR( 0L, AE_UNKNOWN_SYNTAX_PARSER_CONVERT );
            return 0L;
        }
        /* if there is a change in type, then parse the previous buffer */
        if ( ( previous != convert ) && ( previous != 0 ) )
        {
            if ( parse_buffer( final_prog, previous, buffer ) == -1 )
            {
                interpret_cleanup( &final_prog );
                return 0L;
            }

            /* clear the buffer for new characters coming in */
            buffer_size = 0;
            memory_erase( buffer, VARIABLE_WIDTH );
        }

        /* add the character to the buffer */
        buffer[buffer_size++] = temp;

        /* if the buffer gets to big, it's a problem */
        if ( buffer_size == ( VARIABLE_WIDTH -  1 ) )
        {
            interpret_cleanup( &final_prog );
            ( void )APESCRIPT_ERROR( 0L, AE_MAXIMUM_SCRIPT_SIZE_REACHED );
            return 0L;
        }

        /* track the previous type */
        previous = convert;
    }
    /* handle the last type case at the end of the input file */
    if ( parse_buffer( final_prog, previous, buffer ) == -1 )
    {
        interpret_cleanup( &final_prog );
        return 0L;
    }
    {
        n_byte	local_numbers[SIZEOF_NUMBER_WRITE];
        n_uint   loop_sizeof_number;
        /* this is the one special case for direct writing as the original stamp size was allowed */
        io_int_to_bytes( final_prog->binary_code->location, final_prog->binary_code->data ); /* write the basic size header */
        end_loop = number_num;
        loop = 1;
        io_int_to_bytes( number_num, local_numbers ); /* write the number of numbers */
        loop_sizeof_number = 0;
        while ( loop_sizeof_number < SIZEOF_NUMBER_WRITE )
        {
            if ( io_file_write( final_prog->binary_code, local_numbers[loop_sizeof_number] ) == -1 )
            {
                interpret_cleanup( &final_prog );
                return 0L;
            }
            loop_sizeof_number++;
        }
        while ( loop < end_loop )
        {
            io_int_to_bytes( ( final_prog->number_buffer[loop] ), local_numbers ); /* write the numbers */
            loop_sizeof_number = 0;
            while ( loop_sizeof_number < SIZEOF_NUMBER_WRITE )
            {
                if ( io_file_write( final_prog->binary_code, local_numbers[loop_sizeof_number] ) == -1 )
                {
                    interpret_cleanup( &final_prog );
                    return 0L;
                }
                loop_sizeof_number++;
            }
            loop++;
        }
    }
#ifdef SCRIPT_DEBUG
    local_var_codes = variables;
#endif
    return final_prog;
}

