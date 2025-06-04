/****************************************************************

 interpret.c

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

/*! \file   interpret.c
 *  \brief  This handles the interpretation end of ApeScript (i.e. after parsing).
    Unlike the parsing code that should be run only once in general use, the
    interpretation code is run multiple times in a simulation cycle. Thus slightly
    more care must be taken in optimizing the interpret code.
 */

#include "../toolkit/toolkit.h"

#include "script.h"

/**
 * Makes sure all the data associated with the interpreter is freed etc.
 * @param individual pointer to the interpreter structure that is being executed.
 * @param eval pointer execution points being executed.
 * @param location the location in the execution points.
 * @return minus one on failure, zero on success.
 */
static	n_int	interpret_braces( n_individual_interpret *individual, n_byte *eval, n_int location )
{
    n_int		local_b_count;

    NA_ASSERT( individual, "individual NULL" );

    local_b_count = individual->braces_count;
    if ( location == -1 )
    {
        if ( local_b_count == 0 )
        {
            return APESCRIPT_ERROR( individual, AE_TOO_MANY_CLOSE_BRACES );
        }
        individual->braces_count--;
    }
    else
    {
        n_uint		loop = 0;
        n_byte		*local_evaluate;
        if ( local_b_count == BRACES_MAX )
        {
            return APESCRIPT_ERROR( individual, AE_MAXIMUM_BRACES_REACHED );
        }
        local_evaluate = individual->braces[ local_b_count ].evaluate;
        while ( loop < SIZE_OF_EVALUATE )
        {
            n_byte	eval_val = 0;
            if ( eval != 0L )
            {
                eval_val = eval[loop];
            }
            local_evaluate[ loop++ ] = eval_val;
        }
        individual->braces[ local_b_count ].braces_start = location;
        individual->braces_count++;
    }
    return 0;
}

/**
 * Makes sure all the data associated with the interpreter is freed etc.
 * @param code pointer to the interpreter structure that is being executed.
 * @param evaluate pointer execution points being executed.
 * @param number the pointer to the result of the execution block.
 * @param end_char the end character expected.
 * @return minus one on failure, otherwise the number of execution points to advance.
 */
static n_int interpret_apply( n_interpret *code, n_individual_interpret *individual, n_byte *evaluate, n_int *number, n_byte end_char )
{
    n_int	val_a, val_b, val_c;

    NA_ASSERT( code, "code NULL" );
    NA_ASSERT( evaluate, "evaluate NULL" );
    NA_ASSERT( number, "number NULL" );

    if ( code == 0L )
    {
        return SHOW_ERROR( "No code provided" );
    }
    if ( evaluate == 0L )
    {
        return SHOW_ERROR( "Nothing to evaluate" );
    }
    if ( number == 0L )
    {
        return SHOW_ERROR( "No numbers provided" );
    }

    if ( code->sc_output( code, individual, evaluate, &val_a ) == -1 )
    {
        return APESCRIPT_ERROR( individual, AE_FIRST_VALUE_FAILED );
    }
    if ( evaluate[2] == end_char )
    {
        *number = val_a;
        return 3;
    }
    if ( evaluate[2] != APESCRIPT_OPERATOR )
    {
        return APESCRIPT_ERROR( individual, AE_UNKNOWN_SYNTAX_MISSING_EQUALS );
    }
    if ( code->sc_output( code, individual, &evaluate[4], &val_b ) == -1 )
    {
        return APESCRIPT_ERROR( individual, AE_SECOND_VALUE_FAILED );
    }
    val_c = val_a - val_b;
    switch ( evaluate[3] )
    {
    case SYNTAX_MINUS:
        *number = val_c;
        break;
    case SYNTAX_ADDITION:
        *number = ( val_a + val_b );
        break;
    case SYNTAX_MULTIPLY:
        *number = ( val_a * val_b );
        break;
    case SYNTAX_AND:
        *number = ( val_a & val_b );
        break;
    case SYNTAX_XOR:
        *number = ( val_a ^ val_b );
        break;
    case SYNTAX_OR:
        *number = ( val_a | val_b );
        break;
    case SYNTAX_GREATER_THAN:
        *number = ( ( 0 - val_c ) < 0 );
        break;
    case SYNTAX_LESS_THAN:
        *number = ( val_c < 0 );
        break;
    case SYNTAX_EQUAL_TO:
        *number = ( val_c == 0 );
        break;
    case SYNTAX_NOT_EQUAL_TO:
        *number = ( val_c != 0 );
        break;
    case SYNTAX_CONDITIONAL_AND:
        *number = ( val_a && val_b );
        break;
    case SYNTAX_CONDITIONAL_OR:
        *number = ( val_a || val_b );
        break;
    case SYNTAX_DIVISION:
        if ( val_b == 0 )
        {
            *number = 0;
        }
        else
        {
            *number = ( val_a / val_b );
        }
        break;
    case SYNTAX_MODULUS:
        if ( val_b == 0 )
        {
            *number = 0;
        }
        else
        {
            *number = ( val_a % val_b );
        }
        break;
    case SYNTAX_GREATER_EQUAL:
        *number = ( ( 0 - val_c ) <= 0 );
        break;
    case SYNTAX_LESS_EQUAL:
        *number = ( val_c <= 0 );
        break;
    case SYNTAX_BITSHIFT_RIGHT:
        val_b = 0 - val_b;
    case SYNTAX_BITSHIFT_LEFT:
        if ( val_b == 0 )
        {
            *number = val_a;
        }
        else
        {
            if ( val_b < 0 )
            {
                *number = val_a << val_b;
            }
            else
            {
                val_b = 0 - val_b;
                *number = val_a >> val_b;
            }
        }
        break;
    default:
        return APESCRIPT_ERROR( individual, AE_UNKNOWN_SYNTAX_NO_COMMAND );
        break;
    }

    if ( evaluate[6] == end_char )
    {
        return 7;
    }
    return APESCRIPT_ERROR( individual, AE_WRONG_END );
}

/**
 * Makes sure all the data associated with the interpreter is freed etc.
 * @param code pointer to the interpreter structure that is being executed.
 * @param value pointer execution points being executed.
 * @param location the location in the execution points.
 * @return minus one on failure, otherwise the number of execution points to advance.
 */
static n_int interpret_syntax( n_interpret *code, n_individual_interpret *individual, n_byte *value, n_int location )
{
    n_byte	first_value;
    n_byte	second_value;
    n_int	output_number = 0;

    NA_ASSERT( code, "code NULL" );
    NA_ASSERT( value, "value NULL" );

    if ( code == 0L )
    {
        return SHOW_ERROR( "No code provided" );
    }
    if ( value == 0L )
    {
        return SHOW_ERROR( "No values provided" );
    }

    first_value = value[0];
    second_value = value[1];

    if ( first_value == APESCRIPT_CLOSE_BRACE ) /* what do you do with the tailing brace? */
    {
        n_brace	*local_brace;
        n_int	brace_value = ( individual->braces_count ) - 1;
        if ( brace_value < 0 )
        {
            return APESCRIPT_ERROR( individual, AE_TOO_MANY_CLOSE_BRACES );
        }
        local_brace = &( individual->braces[brace_value] );
        if ( local_brace->evaluate[0] == 0 ) /* exit if */
        {
            if ( interpret_braces( individual, 0L, -1 ) == -1 )
            {
                return -1; /* Enough error information provided by this point */
            }
            SC_DEBUG_STRING( individual->interpret_data, "}" );
            SC_DEBUG_DOWN( individual->interpret_data );
            SC_DEBUG_NEWLINE( individual->interpret_data );
            return 1;
        }
        return 0; /* exit while and run function */
    }
    if ( first_value != APESCRIPT_TEXT )
    {
        return APESCRIPT_ERROR( individual, AE_LINE_START_INCORRECT );
    }

    if ( ( second_value > VARIABLE_IF ) && ( second_value <= code->input_greater ) )
    {
        return APESCRIPT_ERROR( individual, AE_OUTPUT_SET_AS_INPUT_VARIABLE );
    }

    if ( VARIABLE_SPECIAL( second_value, code ) ) /* if/while/function/run( something ){} */
    {
        n_int	return_value;
        n_int	error_value = -1;
        if ( value[2] != APESCRIPT_OPEN_BRACKET )
        {
            return APESCRIPT_ERROR( individual, AE_IF_WHILE_NOT_FOLLOWED_BY_BRACKET );
        }
        return_value = interpret_apply( code, individual, &value[3], &output_number, APESCRIPT_CLOSE_BRACKET );
        if ( return_value == -1 )
        {
            return -1; /* Enough information presented by this point */
        }
        if ( second_value == VARIABLE_FUNCTION || second_value == VARIABLE_RUN )
        {
            if ( value[3] != APESCRIPT_TEXT )
            {
                return APESCRIPT_ERROR( individual, AE_FUNCTION_ISNT_VARIABLE );
            }
            if ( return_value != 3 )
            {
                return APESCRIPT_ERROR( individual, AE_NON_FUNCTION_APPLIED );
            }
        }
        else
        {
            SC_DEBUG_STRING( individual->interpret_data, scdebug_variable( second_value ) );
            SC_DEBUG_STRING( individual->interpret_data, " ( ) {" );
            if ( output_number == 0 )
            {
                SC_DEBUG_STRING( individual->interpret_data, " }" );
            }
            else
            {
                SC_DEBUG_UP( individual->interpret_data );
            }
            SC_DEBUG_NEWLINE( individual->interpret_data );
        }
        if ( second_value == VARIABLE_FUNCTION )
        {
            if ( output_number != 0 )
            {
                return APESCRIPT_ERROR( individual, AE_FUNCTION_DEFINED_PRIOR );
            }
        }
        if ( second_value == VARIABLE_RUN )
        {
            if ( ( output_number < 1 ) || ( output_number > 0xFFFF ) )
            {
                return APESCRIPT_ERROR( individual, AE_FUNCTION_OUT_OF_RANGE );
            }
            if ( value[3 + return_value] != APESCRIPT_SEMICOLON )
            {
                return APESCRIPT_ERROR( individual, AE_WITHOUT_SEMICOLON );
            }
            {
                n_byte	function_location[SIZE_OF_EVALUATE] = {APESCRIPT_FUNCTION, 0};
                n_byte	*location_write = ( n_byte * )&function_location[1];
                n_int	continuation = return_value + 4 + location;
                io_int_to_bytes( output_number, location_write );
                location_write = ( n_byte * )&function_location[1 + SIZEOF_NUMBER_WRITE];
                io_int_to_bytes( continuation, location_write );

                NA_ASSERT( ( n_byte * )function_location, "eval function_location" );
                if ( interpret_braces( individual, ( n_byte * )function_location, 0 ) == -1 )
                {
                    return -1; /* Enough information presented by this point */
                }
            }
            SC_DEBUG_STRING( individual->interpret_data, "run( " );
            SC_DEBUG_STRING( individual->interpret_data, scdebug_variable( value[4] ) );
            SC_DEBUG_STRING( individual->interpret_data, " ){" );
            SC_DEBUG_UP( individual->interpret_data );
            SC_DEBUG_NEWLINE( individual->interpret_data );
            return 0; /* want to trigger while check */
        }
        /* if the future code will contain if()run(); then the if should be checked here */
        if ( value[3 + return_value] != APESCRIPT_OPEN_BRACE )
        {
            return APESCRIPT_ERROR( individual, AE_WITHOUT_OPEN_BRACE );
        }
        if ( second_value == VARIABLE_FUNCTION )
        {
            if ( code->sc_input( individual, value[4], ( 4 + return_value + location ) ) == -1 )
            {
                return APESCRIPT_ERROR( individual, AE_FUNCTION_SETTING_FAILED );
            }
            if ( value[4] == code->main_entry )
            {
                if ( interpret_braces( individual, 0L, 0 ) == -1 )
                {
                    return APESCRIPT_ERROR( individual, AE_ERROR_STARTING_MAIN );
                }
                individual->main_status = MAIN_RUN;
                SC_DEBUG_STRING( individual->interpret_data, "function( " );
                SC_DEBUG_STRING( individual->interpret_data, scdebug_variable( value[4] ) );
                SC_DEBUG_STRING( individual->interpret_data, " ){" );
                SC_DEBUG_UP( individual->interpret_data );
                SC_DEBUG_NEWLINE( individual->interpret_data );
                return 3 + 4; /* tF(tf){*/
            }
            if ( individual->main_status != MAIN_NOT_RUN )
            {
                return APESCRIPT_ERROR( individual, AE_CODE_AFTER_MAIN );
            }
        }
        /* if the result is zero find the end of the correctly nested } */
        if ( output_number == 0 )
        {
            n_int	loop = return_value + 4;
            n_int	braces_open = 1;
            n_int   remaining_bytes = ( n_int )code->binary_code->location;
            do
            {
                n_byte	actual_value = value[loop++];
                /* get actual point, avoid text variable numerical reference to { } */
                if ( CODE_VALUE_REQUIRED( actual_value ) )
                {
                    loop++;
                }
                if ( actual_value == APESCRIPT_OPEN_BRACE )
                {
                    braces_open ++;
                }
                if ( actual_value == APESCRIPT_CLOSE_BRACE )
                {
                    braces_open --;
                }
                if ( ( loop + location ) > remaining_bytes )
                {
                    return APESCRIPT_ERROR( individual, AE_NO_CLOSE_BRACE_TO_END_OF_FILE );
                }
            }
            while ( braces_open != 0 );
            return loop;
        }
        /* evaulate accordingly */
        if ( second_value == VARIABLE_IF )
        {
            error_value = interpret_braces( individual, 0L, 0 );
        }
        if ( second_value == VARIABLE_WHILE )
        {
            NA_ASSERT( &value[3], "eval value[3]" );
            error_value = interpret_braces( individual, &value[3], location + return_value + 4 );
        }
        if ( error_value == -1 )
        {
            return -1; /* Enough information presented by this point */
        }
        return return_value + 4;
    }
    if ( individual->main_status == MAIN_NOT_RUN )
    {
        return APESCRIPT_ERROR( individual, AE_CODE_OUTSIDE_FUNCTION );
    }
    if ( VARIABLE_INPUT( second_value, code ) ) /* x = y + z; */
    {
        n_int	return_value;
        if ( ( value[2] != APESCRIPT_OPERATOR ) || ( value[3] != SYNTAX_EQUALS ) )
        {
            return APESCRIPT_ERROR( individual, AE_INPUT_VARIABLE_WITHOUT_EQUALS );
        }
        return_value = interpret_apply( code, individual, &value[4], &output_number, APESCRIPT_SEMICOLON );
        if ( return_value == -1 )
        {
            return -1; /* Enough information presented by this point */
        }
        if ( code->sc_input( individual, second_value, output_number ) == -1 )
        {
            return APESCRIPT_ERROR( individual, AE_ASSIGN_VALUE_FAILED );
        }
        SC_DEBUG_STRING( individual->interpret_data, scdebug_variable( second_value ) );
        SC_DEBUG_STRING( individual->interpret_data, " = " );
        SC_DEBUG_NUMBER( individual->interpret_data, output_number );
        SC_DEBUG_STRING( individual->interpret_data, " ;" );
        SC_DEBUG_NEWLINE( individual->interpret_data );
        return return_value + 4;
    }
    return APESCRIPT_ERROR( individual, AE_UNKNOWN_SYNTAX_FROM_INTERPRET );
}

/**
 * The start of the interpreter cycle.
 * @param interp pointer to the interpreter structure that is being executed.
 */
static void interpret_start( n_interpret *interp, n_individual_interpret *individual )
{
    n_byte	*local_data     = interp->binary_code->data;
    n_int	*local_number   = interp->number_buffer;
    n_int	*local_variable = individual->variable_references;
    n_int	 end_loop = io_bytes_to_int( local_data );
    n_byte	*start_numbers = &local_data[end_loop];
    n_int	 local_number_num = io_bytes_to_int( start_numbers );
    n_int	 loop = 0;
    individual->main_status = MAIN_NOT_RUN;
    individual->braces_count = 0;
    while ( loop++ < BRACES_MAX )
    {
        ( void )interpret_braces( individual, 0L, 0 ); /* No errors in this initialisation */
    }
    individual->braces_count = 0;
    loop = 1;
    local_number[0] = 0;
    while ( loop < local_number_num )
    {
        local_number[loop] = ( n_int )io_bytes_to_int( &start_numbers[loop * ( n_int )SIZEOF_NUMBER_WRITE] );
        loop++;
    }
    loop = 0;
    while ( loop < ( VARIABLE_MAX ) )
    {
        local_variable[loop++] = 0;
    }
}

/**
 * Makes sure all the data associated with the interpreter is freed etc.
 * @param interp pointer to the interpreter structure that is being executed.
 * @return zero on success, minus one on failure.
 */
static n_int	interpret_code( n_interpret *interp, n_individual_interpret *individual )
{
    n_byte	*local_data  = interp->binary_code->data;
    n_int	 loop        = SIZEOF_NUMBER_WRITE;
    n_int	 cycle_count = 0;
    n_int	 end_loop    = io_bytes_to_int( local_data );

    if ( individual->interpret_location != 0 )
    {
        loop = individual->interpret_location;
        individual->interpret_location = 0;
    }

    /* this is the interpret loop */
    do
    {
        n_int	result = interpret_syntax( interp, individual, &local_data[loop], loop );
        if ( result == -1 )
        {
            return -1; /* Enough information presented by this point */
        }

        if ( result != 0 )
        {
            loop += result;
        }
        else     /* This is the while check conditional */
        {
            n_brace *local_brace;
            n_int	  brace_number = ( individual->braces_count - 1 );
            n_byte	  first_evaluate;
            if ( brace_number < 0 )
            {
                return APESCRIPT_ERROR( individual, AE_TOO_MANY_CLOSE_BRACES );
            }
            local_brace = &( individual->braces[brace_number] );
            first_evaluate = local_brace->evaluate[0];
            if ( first_evaluate == APESCRIPT_RUN || first_evaluate == APESCRIPT_FUNCTION ) /* check the function run */
            {
                if ( first_evaluate == APESCRIPT_FUNCTION )
                {
                    local_brace->evaluate[0] = APESCRIPT_RUN;
                    loop = io_bytes_to_int( &( local_brace->evaluate[1] ) );
                }
                else   /* end of the run function , put back to where 'run' is called */
                {
                    loop = io_bytes_to_int( &( local_brace->evaluate[1 + SIZEOF_NUMBER_WRITE] ) );
                    if ( interpret_braces( individual, 0L, -1 ) == -1 ) /* remove the run function from braces */
                    {
                        return -1; /* Enough information presented by this point */
                    }
                    SC_DEBUG_STRING( individual->interpret_data, "}" );
                    SC_DEBUG_DOWN( individual->interpret_data );
                    SC_DEBUG_NEWLINE( individual->interpret_data );
                }
            }
            else
            {
                n_int	  return_value = 0;

                if ( interpret_apply( interp, individual, local_brace->evaluate, &return_value, APESCRIPT_CLOSE_BRACKET ) == -1 )
                {
                    return -1; /* Enough information presented by this point */
                }
                if ( return_value == 0 )
                {
                    if ( interpret_braces( individual, 0L, -1 ) == -1 )
                    {
                        return -1; /* Enough information presented by this point */
                    }
                    SC_DEBUG_STRING( individual->interpret_data, "}" );
                    SC_DEBUG_DOWN( individual->interpret_data );
                    SC_DEBUG_NEWLINE( individual->interpret_data );
                    loop++;
                }
                else
                {
                    loop = local_brace->braces_start;
                }
            }
        }

        cycle_count++;
    }
    while ( ( loop < end_loop ) && ( cycle_count < CYCLE_COUNT_RESET ) && ( individual->leave == 0 ) );

    if ( ( individual->leave != 0 ) || ( cycle_count == CYCLE_COUNT_RESET ) )
    {
        individual->interpret_location = loop;
    }
    else
    {
        if ( individual->main_status == MAIN_NOT_RUN )
        {
            return APESCRIPT_ERROR( individual, AE_NO_MAIN_CODE );
        }

        SC_DEBUG_OFF( individual->interpret_data ); /* turn off debugging after first cycle */
    }

    return 0;
}

void interpret_individual( n_individual_interpret *individual )
{
    individual->interpret_location = 0;
    individual->leave = 0;
    individual->localized_leave = 0;
}

/**
 Makes sure all the data associated with the interpreter is freed etc.
 @param to_clean The pointer to the n_interpret struct that is being expunged.
 */
void interpret_cleanup( n_interpret **to_clean )
{
#ifdef SCRIPT_DEBUG
    scdebug_file_cleanup();
#endif
    if ( *to_clean == 0L )
    {
        return;
    }
    if ( ( *to_clean )->binary_code != 0L )
    {
        io_file_free( &( ( *to_clean )->binary_code ) );
    }
    memory_free( ( void ** )to_clean );
}

/**
 * This processes a single cycle of the ApeScript interpreter.
 * @param code the ApeScript code to be executed.
 * @param exit_offset if greater than minus one, the value entry to indicate exiting interpreter.
 * @param structure the structure to be passed into the start and end functions.
 * @param start the function to be run at the start of the ApeScript cycle.
 * @param end the function to be run at the end of the ApeScript cycle.
 * @return -1 in error case, 0 in leave and don't cycle back, 1 in leave and continue to cycle back.
 */
n_int interpret_cycle( n_interpret *code, n_individual_interpret *individual, n_int exit_offset,
                       void *structure, void *data,
                       script_external *start, script_external *end )
{
    if ( code == 0L )
    {
        return 0;
    }

    individual->interpret_data = data;

    if ( individual->localized_leave )
    {
        individual->localized_leave--;
    } /* the localized_leave = 1 case is where the interpreter was left initially */
    if ( individual->localized_leave )
    {
        return 1;
    }

    if ( individual->interpret_location == 0 )
    {
        interpret_start( code, individual );
        if ( start != 0L )
        {
            ( *start )( individual, structure, data );
        }
    }

    if ( interpret_code( code, individual ) == -1 )
    {
        return -1;
    }

    if ( individual->interpret_location == 0 )
    {
        if ( ( code != 0L ) && ( end != 0L ) )
        {
            ( *end )( individual, structure, data );
        }
    }

    individual->localized_leave = individual->leave;

    if ( exit_offset > -1 )
    {
        n_int *variables = individual->variable_references;
        if ( variables[exit_offset] == 0 )
        {
            return 1;
        }
    }
    return 0;
}

n_int apescript_error( n_individual_interpret *individual, AE_ENUM value, n_constant_string location, n_int line_number )
{
    n_int    loop = 0;
    AE_ENUM  local_enum;
    n_constant_string local_error;
    do
    {
        local_enum = apescript_errors[loop].enum_value;
        local_error = apescript_errors[loop].error_string;
        if ( value == local_enum )
        {
            if ( individual->interpret_data )
            {
                SC_DEBUG_STRING( individual->interpret_data, " [ ERROR : " );
                SC_DEBUG_STRING( individual->interpret_data, local_error );
                SC_DEBUG_STRING( individual->interpret_data, " ]" );
                SC_DEBUG_NEWLINE( individual->interpret_data );
                SC_DEBUG_OFF( individual->interpret_data );
            }
            return SHOW_ERROR_FILE_LINE( local_error, location, line_number );
        }
        loop++;
    }
    while ( ( local_enum != AE_NO_ERROR ) && ( local_error != 0L ) );

    return apescript_error( individual, AE_UNKNOWN_ERROR, location, line_number );
}

