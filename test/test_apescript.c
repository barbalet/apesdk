/****************************************************************

 test_apescript.c

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

#ifndef	_WIN32

#include "../toolkit/toolkit.h"
#include "../script/script.h"

#else

#include "..\toolkit\toolkit.h"
#include "..\script\script.h"

#endif

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

/*
 This is intended to be overwritten by third-party usage where applicable.
 */

enum SECONDARY_APESCRIPT
{
    VARIABLE_RANDOM = ( VARIABLE_IF + 1 ),
    VARIABLE_ESCAPE_PER_SECOND,

    VARIABLE_GET_ONE,
    VARIABLE_GET_TWO,
    VARIABLE_GET_THREE,
    VARIABLE_GET_FOUR,
    VARIABLE_GET_FIVE,
    VARIABLE_GET_SIX,
    VARIABLE_GET_SEVEN,
    VARIABLE_GET_EIGHT,

    VARIABLE_EXTERNAL_ONE,
    VARIABLE_EXTERNAL_TWO,
    VARIABLE_EXTERNAL_THREE,
    VARIABLE_EXTERNAL_FOUR,
    VARIABLE_EXTERNAL_FIVE,
    VARIABLE_EXTERNAL_SIX,

    VARIABLE_PERSIST,

    VARIABLE_ONE,
    VARIABLE_TWO,
    VARIABLE_THREE,
    VARIABLE_FOUR,
    VARIABLE_FIVE,
    VARIABLE_SIX,
    VARIABLE_SEVEN,

    VARIABLE_RANDOM_SEED,

    VARIABLE_EXIT,

    VARIABLE_RUN_EXTERNAL_FUNCTION,

    VARIABLE_ESCAPE,

    VARIABLE_MAIN /* This is a special case, it is the location where the main code starts */
};

#define VARIABLE_READWRITE VARIABLE_EXTERNAL_SIX

#define VARIABLE_FIRST_REAL_ONE VARIABLE_ONE

static variable_string	variable_codes[VARIABLE_MAX] =
{
    /* 0 */ /* special "variables" */
    "function",
    "run",

    /* 2 */
    "while",
    "if",

    /* output only */
    "random",
    "escape_per_second",

    "variable_get_one",
    "variable_get_two",
    "variable_get_three",
    "variable_get_four",
    "variable_get_five",
    "variable_get_six",
    "variable_get_seven",
    "variable_get_eight",

    "external_function_one",
    "external_function_two",
    "external_function_three",
    "external_function_four",
    "external_function_five",
    "external_function_six",

    /* input and output */

    "persist",

    "variable_one",
    "variable_two",
    "variable_three",
    "variable_four",
    "variable_five",
    "variable_six",
    "variable_seven",

    "random_seed",

    "exit",

    "run_external_function",

    "escape",

    /* main call */
    "main"
};

#define FUNCTION_ONE_TRIGGER_NUMBER   (65001)
#define FUNCTION_TWO_TRIGGER_NUMBER   (65002)
#define FUNCTION_THREE_TRIGGER_NUMBER (65003)
#define FUNCTION_FOUR_TRIGGER_NUMBER  (65004)
#define FUNCTION_FIVE_TRIGGER_NUMBER  (65005)
#define FUNCTION_SIX_TRIGGER_NUMBER   (65006)

#define FRACTION_OF_SECOND      (6)


static n_byte2  random_seed[2];
static n_byte   random_populated = 0;
static n_int	persist_value = 0;
static n_uint   random_noise = 0x6d73e3a3;

static void commands_random_seed( n_int seed_value )
{
    random_populated = 1;

    if ( seed_value < 0 )
    {
        n_uint unsignedvalue = time( NULL );
        switch ( random_noise % 2 )
        {
        case 0:
            random_noise   ^= ( unsignedvalue ^ 0x47c23e19 );
            break;
        case 1:
            random_noise   ^= ( unsignedvalue ^ 0x9ef1de93 );
            break;
        case 2:
            random_noise   ^= ( unsignedvalue ^ 0x2e97c735 );
            break;
        }
        random_seed[0] ^= ( random_noise >> 16 ) & 0xFFFF;
        random_seed[1] ^=  random_noise & 0xFFFF;
    }
    else
    {
        n_uint  seed = ABS( seed_value );

        seed = ( seed * 2999 ) & 0xFFFFFFFF;

        random_seed[0] = ( seed >> 16 ) & 0xFFFF;
        random_seed[1] =  seed & 0xFFFF;
    }
}

static n_byte2 commands_handle_random()
{
    if ( random_populated == 0 )
    {
        commands_random_seed( -1 );
    }
    return math_random( random_seed );
}

/* notification here is optional and it can be done asynchronously */

static void commands_notify_done( n_int *notification )
{
    notification[0] = 0;
}

static void commands_process_run( n_int value, n_int *notification )
{
    /* do not create any additional noise if the values aren't found */
    switch ( value )
    {
    case FUNCTION_ONE_TRIGGER_NUMBER:
    case FUNCTION_TWO_TRIGGER_NUMBER:
    case FUNCTION_THREE_TRIGGER_NUMBER:
    case FUNCTION_FOUR_TRIGGER_NUMBER:
    case FUNCTION_FIVE_TRIGGER_NUMBER:
    case FUNCTION_SIX_TRIGGER_NUMBER:
        /* best not to add a new line for the debug format */
        commands_notify_done( notification );
        break;
    }
}

n_int commands_input( void *vindividual, n_byte kind, n_int value )
{
    n_individual_interpret *individual = ( n_individual_interpret * )vindividual;
    n_int *local_vr = individual->variable_references;

    if ( kind > VARIABLE_READWRITE )
    {

        if ( kind == VARIABLE_PERSIST )
        {
            persist_value = value;
        }
        else
        {
            local_vr[kind - VARIABLE_FIRST_REAL_ONE] = value;
        }


        if ( kind == VARIABLE_RUN_EXTERNAL_FUNCTION )
        {
            commands_process_run( value, &local_vr[kind - VARIABLE_FIRST_REAL_ONE] );
        }

        if ( kind == VARIABLE_RANDOM_SEED )
        {
            commands_random_seed( value );
        }
        if ( kind == VARIABLE_ESCAPE )
        {
            individual->leave = value;
        }
        return 0;
    }
    return -1; /* where this fails is more important than this failure */
}

n_int commands_output( void *vcode, void *vindividual, n_byte *kind, n_int *number )
{
    n_interpret *code = ( n_interpret * )vcode;
    n_individual_interpret *individual = ( n_individual_interpret * )vindividual;
    n_byte	first_value = kind[0];
    n_byte	second_value = kind[1];
    if ( first_value == 'n' )
    {
        *number = code->number_buffer[second_value];
        return 0;
    }
    if ( ( first_value == 't' ) && ( VARIABLE_SPECIAL( second_value, code ) == 0 ) )
    {
        n_int	*local_vr = individual->variable_references;

        if ( second_value > VARIABLE_READWRITE )
        {
            if ( second_value == VARIABLE_PERSIST )
            {
                *number = persist_value;
            }
            else
            {
                *number = local_vr[second_value - VARIABLE_FIRST_REAL_ONE];

            }
            return 0;
        }
        else
        {
            switch ( second_value )
            {
            case VARIABLE_RANDOM:
                *number = commands_handle_random();
                return 0;

            /* this is a special constant */
            case VARIABLE_ESCAPE_PER_SECOND:
                *number = FRACTION_OF_SECOND;
                return 0;

            case VARIABLE_GET_ONE:
                *number = 1; /* this could be any value */
                return 0;
            case VARIABLE_GET_TWO:
                *number = 2; /* this could be any value */
                return 0;
            case VARIABLE_GET_THREE:
                *number = 3; /* this could be any value */
                return 0;
            case VARIABLE_GET_FOUR:
                *number = 4; /* this could be any value */
                return 0;
            case VARIABLE_GET_FIVE:
                *number = 5; /* this could be any value */
                return 0;
            case VARIABLE_GET_SIX:
                *number = 6; /* this could be any value */
                return 0;
            case VARIABLE_GET_SEVEN:
                *number = 7; /* this could be any value */
                return 0;
            case VARIABLE_GET_EIGHT:
                *number = 8; /* this could be any value */
                return 0;

            /* there are a number of ways of doing this
             in terms of unique execution points
             */
            case VARIABLE_EXTERNAL_ONE:
                *number = FUNCTION_ONE_TRIGGER_NUMBER;
                return 0;
            case VARIABLE_EXTERNAL_TWO:
                *number = FUNCTION_TWO_TRIGGER_NUMBER;
                return 0;
            case VARIABLE_EXTERNAL_THREE:
                *number = FUNCTION_THREE_TRIGGER_NUMBER;
                return 0;
            case VARIABLE_EXTERNAL_FOUR:
                *number = FUNCTION_FOUR_TRIGGER_NUMBER;
                return 0;
            case VARIABLE_EXTERNAL_FIVE:
                *number = FUNCTION_FIVE_TRIGGER_NUMBER;
                return 0;
            case VARIABLE_EXTERNAL_SIX:
                *number = FUNCTION_SIX_TRIGGER_NUMBER;
                return 0;
            }
        }
    }
    return -1; /* where this fails is more important than this failure */
}


static n_interpret *interpret = 0L;

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @%s, %ld\n", error_text, location, line_number );
    return -1;
}


static n_int	test_interpret( n_byte *buff, n_uint len )
{
    n_file	local;

    local . size = len;
    local . location = 0;
    local . data = buff;

    interpret = parse_convert( &local, VARIABLE_MAIN, ( variable_string * )variable_codes );

    if ( interpret == 0L )
    {
        return -1;
    }
    else
    {
        SC_DEBUG_ON( 0L ); /* turn on debugging after script loading */
    }

    interpret->sc_input  = &commands_input;
    interpret->sc_output = &commands_output;

    interpret->input_greater   = VARIABLE_READWRITE;

    return 0;
}

static n_int test_init( n_string interpret_string )
{
    n_file *source_file = io_file_new();

    if ( io_disk_check( interpret_string ) == 0 )
    {
        return -1;
    }
    if ( io_disk_read( source_file, interpret_string ) == FILE_ERROR )
    {
        io_file_free( &source_file );
        return -1;
    }
    if ( test_interpret( source_file->data, source_file->location ) == -1 )
    {
        io_file_free( &source_file );
        return -1;
    }
    io_file_free( &source_file );
    return 0;
}

static void test_close()
{
    interpret_cleanup( &interpret );
}

int main( int argc, char *argv[] )
{
    n_int   return_value = 0;
    n_individual_interpret individual;

    printf( " --- test apescript --- start -----------------------------------------\n" );

    interpret_individual( &individual );

    if ( argc != 2 )
    {
        printf( "ERROR: Single script file string expected\n" );
        return 0;
    }

    if ( test_init( argv[1] ) == -1 )
    {
        printf( "ERROR: Load Script %s failed\n", argv[1] );
        return 0;
    }

    do
    {
        return_value = interpret_cycle( interpret, &individual, VARIABLE_EXIT - VARIABLE_FIRST_REAL_ONE, 0L, 0L, 0L, 0L );
        if ( individual.leave != 0 )
        {
#ifdef COMMAND_LINE_DEBUG
            printf( "..." );
#endif
        }

    }
    while ( return_value == 1 );

    if ( return_value == -1 )
    {
        printf( "ERROR: Script %s Ended with Error\n", argv[1] );
    }

    test_close();

    printf( " --- test apescript ---  end  -----------------------------------------\n" );
    
    if ( return_value == -1 )
    {
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
