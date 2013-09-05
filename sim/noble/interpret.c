/****************************************************************

 interpret.c

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

/*! \file   interpret.c
 *  \brief  This handles the interpretation end of ApeScript (i.e. after parsing).
    Unlike the parsing code that should be run only once in general use, the
    interpretation code is run multiple times in a simulation cycle. Thus slightly
    more care must be taken in optimizing the interpret code.
 */

/*NOBLEMAKE DEL=""*/
#include "noble.h"
/*NOBLEMAKE END=""*/
static	n_int	interpret_braces(n_interpret * code, n_byte * eval, n_int location)
{
    n_int		local_b_count;
    
    NA_ASSERT(code, "code NULL");
    NA_ASSERT(eval, "eval NULL");
    
    local_b_count = code->braces_count;
    if(location == -1)
    {
        if(local_b_count == 0)
        {
            return io_apescript_error(AE_TOO_MANY_CLOSE_BRACES);
        }
        code->braces_count--;
    }
    else
    {
        n_uint		loop = 0;
        n_byte		*local_evaluate;
        if(local_b_count == BRACES_MAX)
        {
            return io_apescript_error(AE_MAXIMUM_BRACES_REACHED);
        }
        local_evaluate = code->braces[ local_b_count ].evaluate;
        while(loop < SIZE_OF_EVALUATE)
        {
            n_byte	eval_val = 0;
            if(eval != 0L)
            {
                eval_val = eval[loop];
            }
            local_evaluate[ loop++ ] = eval_val;
        }
        code->braces[ local_b_count ].braces_start = location;
        code->braces_count++;
    }
    return 0;
}

static n_int interpret_apply(n_interpret * code, n_byte * evaluate, n_int * number, n_byte end_char)
{
    n_int	val_a, val_b, val_c;
    
    NA_ASSERT(code, "code NULL");
    NA_ASSERT(evaluate, "evaluate NULL");
    NA_ASSERT(number, "number NULL");
    
    if (code == 0L) return SHOW_ERROR("No code provided");
    if (evaluate == 0L) return SHOW_ERROR("Nothing to evaluate");
    if (number == 0L) return SHOW_ERROR("No numbers provided");
    
    
    if(code->sc_output(code,evaluate,&val_a) == -1)
    {
        return io_apescript_error(AE_FIRST_VALUE_FAILED);
    }
    if(evaluate[2] == end_char)
    {
        *number = val_a;
        return 3;
    }
    if(evaluate[2] != '=')
    {
        return io_apescript_error(AE_UNKNOWN_SYNTAX_MISSING_EQUALS);
    }
    if(code->sc_output(code,&evaluate[4],&val_b) == -1)
    {
        return io_apescript_error(AE_SECOND_VALUE_FAILED);
    }
    val_c = val_a - val_b;
    switch(evaluate[3])
    {
    case SYNTAX_MINUS:
        *number = val_c;
        break;
    case SYNTAX_ADDITION:
        *number = (val_a + val_b);
        break;
    case SYNTAX_MULTIPLY:
        *number = (val_a * val_b);
        break;
    case SYNTAX_AND:
        *number = (val_a & val_b);
        break;
    case SYNTAX_XOR:
        *number = (val_a ^ val_b);
        break;
    case SYNTAX_OR:
        *number = (val_a | val_b);
        break;
    case SYNTAX_GREATER_THAN:
        *number = ((0 - val_c)<0);
        break;
    case SYNTAX_LESS_THAN:
        *number = (val_c < 0);
        break;
    case SYNTAX_EQUAL_TO:
        *number = (val_c == 0);
        break;
    case SYNTAX_NOT_EQUAL_TO:
        *number = (val_c != 0);
        break;
    case SYNTAX_CONDITIONAL_AND:
        *number = (val_a && val_b);
        break;
    case SYNTAX_CONDITIONAL_OR:
        *number = (val_a || val_b);
        break;
    case SYNTAX_DIVISION:
        if (val_b == 0)
            *number = 0;
        else
            *number = (val_a / val_b);
        break;
    case SYNTAX_MODULUS:
        if (val_b == 0)
            *number = 0;
        else
            *number = (val_a % val_b);
        break;
    case SYNTAX_GREATER_EQUAL:
        *number = ((0 - val_c) <= 0);
        break;
    case SYNTAX_LESS_EQUAL:
        *number = (val_c <= 0);
        break;
    case SYNTAX_BITSHIFT_RIGHT:
        val_b = 0 - val_b;
    case SYNTAX_BITSHIFT_LEFT:
        if (val_b == 0)
        {
            *number = val_a;
        }
        else
        {
            if (val_b < 0)
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
        return io_apescript_error(AE_UNKNOWN_SYNTAX_NO_COMMAND);
        break;
    }

    if(evaluate[6] == end_char)
    {
        return 7;
    }
    return io_apescript_error(AE_WRONG_END);
}

static n_int interpret_syntax(n_interpret * code, n_byte * value, n_int location)
{
    n_byte	first_value;
    n_byte	second_value;
    n_int	output_number = 0;
    
    NA_ASSERT(code, "code NULL");
    NA_ASSERT(value, "value NULL");
    
    if (code == 0L) return SHOW_ERROR("No code provided");
    if (value == 0L) return SHOW_ERROR("No values provided");
    
    first_value = value[0];
    second_value = value[1];

    if(first_value == '}')  /* what do you do with the tailing brace? */
    {
        n_brace	*local_brace;
        n_int	brace_value = (code->braces_count) - 1;
        if(brace_value < 0 )
        {
            return io_apescript_error(AE_TOO_MANY_CLOSE_BRACES);
        }
        local_brace = &(code->braces[brace_value]);
        if(local_brace->evaluate[0] == 0)  /* exit if */
        {
            if(interpret_braces(code,0L,-1) == -1)
            {
                return -1; /* Enough error information provided by this point */
            }
            SC_DEBUG_STRING("}");
            SC_DEBUG_DOWN;
            SC_DEBUG_NEWLINE;
            return 1;
        }
        return 0; /* exit while and run function */
    }
    if(first_value != 't')
    {
        return io_apescript_error(AE_LINE_START_INCORRECT);
    }

    if((second_value > VARIABLE_IF)&&(second_value <= code->input_greater))
    {
        return io_apescript_error(AE_OUTPUT_SET_AS_INPUT_VARIABLE);
    }

    if(VARIABLE_SPECIAL(second_value,code))  /* if/while/function/run( something ){} */
    {
        n_int	return_value;
        n_int	error_value = -1;
        if(value[2] != '(')
        {
            return io_apescript_error(AE_IF_WHILE_NOT_FOLLOWED_BY_BRACKET);
        }
        return_value = interpret_apply(code, &value[3], &output_number, ')');
        if(return_value == -1)
        {
            return -1; /* Enough information presented by this point */
        }
        if(second_value == VARIABLE_FUNCTION || second_value == VARIABLE_RUN)
        {
            if(value[3] != 't')
            {
                return io_apescript_error(AE_FUNCTION_ISNT_VARIABLE);
            }
            if(return_value != 3)
            {
                return io_apescript_error(AE_NON_FUNCTION_APPLIED);
            }
        }
        else
        {
            SC_DEBUG_STRING(scdebug_variable(second_value));
            SC_DEBUG_STRING(" ( ) {");
            if(output_number == 0)
            {
                SC_DEBUG_STRING(" }");
            }
            else
            {
                SC_DEBUG_UP;
            }
            SC_DEBUG_NEWLINE;
        }
        if(second_value == VARIABLE_FUNCTION)
        {
            if(output_number != 0)
            {
                return io_apescript_error(AE_FUNCTION_DEFINED_PRIOR);
            }
        }
        if(second_value == VARIABLE_RUN)
        {
            if((output_number < 1) || (output_number > 0xFFFF))
            {
                return io_apescript_error(AE_FUNCTION_OUT_OF_RANGE);
            }
            if(value[3 + return_value] != ';')
            {
                return io_apescript_error(AE_WITHOUT_SEMICOLON);
            }
            {
                n_byte	function_location[SIZE_OF_EVALUATE] = {'f',0};
                n_byte	*location_write =(n_byte *)&function_location[1];
                n_int	continuation = return_value + 4 + location;
                io_int_to_bytes(output_number, location_write);
                location_write = (n_byte *)&function_location[1 + SIZEOF_NUMBER_WRITE];
                io_int_to_bytes(continuation,location_write);
                if(interpret_braces(code, (n_byte *)function_location, 0) == -1)
                {
                    return -1; /* Enough information presented by this point */
                }
            }
            SC_DEBUG_STRING("run( ");
            SC_DEBUG_STRING(scdebug_variable(value[4]));
            SC_DEBUG_STRING(" ){");
            SC_DEBUG_UP;
            SC_DEBUG_NEWLINE;
            return 0; /* want to trigger while check */
        }
        /* if the future code will contain if()run(); then the if should be checked here */
        if(value[3 + return_value] != '{')
        {
            return io_apescript_error(AE_WITHOUT_OPEN_BRACE);
        }
        if(second_value == VARIABLE_FUNCTION)
        {
            if(code->sc_input(code, value[4], (4 + return_value + location) ) == -1)
            {
                return io_apescript_error(AE_FUNCTION_SETTING_FAILED);
            }
            if(value[4] == code->main_entry)
            {
                if(interpret_braces(code,0L,0) == -1)
                {
                    return io_apescript_error(AE_ERROR_STARTING_MAIN);
                }
                code->main_status = MAIN_RUN;
                SC_DEBUG_STRING("function( ");
                SC_DEBUG_STRING(scdebug_variable(value[4]));
                SC_DEBUG_STRING(" ){");
                SC_DEBUG_UP;
                SC_DEBUG_NEWLINE;
                return 3 + 4; /* tF(tf){*/
            }
            if(code->main_status != MAIN_NOT_RUN)
            {
                return io_apescript_error(AE_CODE_AFTER_MAIN);
            }
        }
        /* if the result is zero find the end of the correctly nested } */
        if(output_number == 0)
        {
            n_int	loop = return_value + 4;
            n_int	braces_open = 1;
            n_int remaining_bytes = code->binary_code->location;
            do
            {
                n_byte	actual_value = value[loop++];
                /* get actual point, avoid text variable numerical reference to { } */
                if(CODE_VALUE_REQUIRED(actual_value))
                    loop++;
                if(actual_value == '{')
                {
                    braces_open ++;
                }
                if(actual_value == '}')
                {
                    braces_open --;
                }
                if((loop + location) > remaining_bytes)
                {
                    return io_apescript_error(AE_NO_CLOSE_BRACE_TO_END_OF_FILE);
                }
            }
            while(braces_open != 0);
            return loop;
        }
        /* evaulate accordingly */
        if(second_value == VARIABLE_IF)
        {
            error_value = interpret_braces(code,0L,0);
        }
        if(second_value == VARIABLE_WHILE)
        {
            error_value = interpret_braces(code,&value[3],location + return_value + 4);
        }
        if(error_value == -1)
        {
            return -1; /* Enough information presented by this point */
        }
        return return_value + 4;
    }
    if(code->main_status == MAIN_NOT_RUN)
    {
        return io_apescript_error(AE_CODE_OUTSIDE_FUNCTION);
    }
    if(VARIABLE_INPUT(second_value, code))  /* x = y + z; */
    {
        n_int	return_value;
        if((value[2] != '=') || (value[3] != SYNTAX_EQUALS))
        {
            return io_apescript_error(AE_INPUT_VARIABLE_WITHOUT_EQUALS);
        }
        return_value = interpret_apply(code, &value[4], &output_number, ';');
        if(return_value == -1)
        {
            return -1; /* Enough information presented by this point */
        }
        if(code->sc_input(code, second_value,output_number) == -1)
        {
            return io_apescript_error(AE_ASSIGN_VALUE_FAILED);
        }
        SC_DEBUG_STRING(scdebug_variable(second_value));
        SC_DEBUG_STRING(" = ");
        SC_DEBUG_NUMBER(output_number);
        SC_DEBUG_STRING(" ;");
        SC_DEBUG_NEWLINE;
        return return_value + 4;
    }
    return io_apescript_error(AE_UNKNOWN_SYNTAX_FROM_INTERPRET);
}

static void interpret_start(n_interpret * interp)
{
    n_byte	*local_data     = interp->binary_code->data;
    n_int	*local_number   = interp->number_buffer;
    n_int	*local_variable = interp->variable_references;
    n_int	 end_loop = io_bytes_to_int(local_data);
    n_byte	*start_numbers = &local_data[end_loop];
    n_int	 local_number_num = io_bytes_to_int(start_numbers);
    n_int	 loop = 0;
    interp->main_status = MAIN_NOT_RUN;
    interp->braces_count = 0;
    while(loop++ < BRACES_MAX)
    {
        (void)interpret_braces(interp,0L,0); /* No errors in this initialisation */
    }
    interp->braces_count = 0;
    loop = 1;
    local_number[0] = 0;
    while(loop < local_number_num)
    {
        local_number[loop] = io_bytes_to_int(&start_numbers[loop*SIZEOF_NUMBER_WRITE]);
        loop++;
    }
    loop = 0;
    while(loop < (VARIABLE_MAX))
    {
        local_variable[loop++] = 0;
    }
}

static n_int	interpret_code(n_interpret * interp)
{
    n_byte	*local_data  = interp->binary_code->data;
    n_int	 loop        = SIZEOF_NUMBER_WRITE;
    n_int	 cycle_count = 0;
    n_int	 end_loop    = io_bytes_to_int(local_data);

    if (interp->location != 0)
    {
        loop = interp->location;
        interp->location = 0;
    }

    /* this is the interpret loop */
    do
    {
        n_int	result = interpret_syntax(interp,&local_data[loop], loop);
        if(result == -1)
        {
            return -1; /* Enough information presented by this point */
        }

        if(result != 0)
        {
            loop += result;
        }
        else     /* This is the while check conditional */
        {
            n_brace * local_brace;
            n_int	  brace_number = (interp->braces_count - 1);
            n_byte	  first_evaluate;
            if(brace_number < 0)
            {
                return io_apescript_error(AE_TOO_MANY_CLOSE_BRACES);
            }
            local_brace = &(interp->braces[brace_number]);
            first_evaluate = local_brace->evaluate[0];
            if(first_evaluate == 'r' || first_evaluate == 'f')  /* check the function run */
            {
                if(first_evaluate == 'f')
                {
                    local_brace->evaluate[0] = 'r';
                    loop = io_bytes_to_int(&(local_brace->evaluate[1]));
                }
                else   /* end of the run function , put back to where 'run' is called */
                {
                    loop = io_bytes_to_int(&(local_brace->evaluate[1 + SIZEOF_NUMBER_WRITE]));
                    if(interpret_braces(interp,0L,-1) == -1)  /* remove the run function from braces */
                    {
                        return -1; /* Enough information presented by this point */
                    }
                    SC_DEBUG_STRING("}");
                    SC_DEBUG_DOWN;
                    SC_DEBUG_NEWLINE;
                }
            }
            else
            {
                n_int	  return_value = 0;

                if(interpret_apply(interp, local_brace->evaluate, &return_value, ')') == -1)
                {
                    return -1; /* Enough information presented by this point */
                }
                if(return_value == 0)
                {
                    if(interpret_braces(interp, 0L, -1) == -1)
                    {
                        return -1; /* Enough information presented by this point */
                    }
                    SC_DEBUG_STRING("}");
                    SC_DEBUG_DOWN;
                    SC_DEBUG_NEWLINE;
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
    while((loop < end_loop) && (cycle_count < CYCLE_COUNT_RESET) && (interp->leave == 0));

    if ((interp->leave != 0) || (cycle_count == CYCLE_COUNT_RESET))
    {
        interp->location = loop;
    }
    else
    {
        if(interp->main_status == MAIN_NOT_RUN)
        {
            return io_apescript_error(AE_NO_MAIN_CODE);
        }

        SC_DEBUG_OFF; /* turn off debugging after first cycle */
    }

    return 0;
}
/**
 Makes sure all the data associated with the interpreter is freed etc.
 @param to_clean The pointer to the n_interpret struct that is being expunged.
 */
void interpret_cleanup(n_interpret * to_clean)
{
#ifdef SCRIPT_DEBUG
    scdebug_file_cleanup();
#endif
    if (to_clean == 0L)
    {
        return;
    }
    if (to_clean->binary_code != 0L)
    {
        io_file_free(to_clean->binary_code);
    }
    if (to_clean->variable_references == 0L)
    {
        io_free(to_clean->variable_references);
    }
    io_free(to_clean);
}

/* returns:
      -1 in error case
       0 in leave and don't cycle back
       1 in leave and continue to cycle back
 */

n_int interpret_cycle(n_interpret * code, n_int exit_offset,
                      void * structure, n_int identifier,
                      script_external * start, script_external * end)
{
    if (code == 0L)
    {
        return 0;
    }
    if (code->localized_leave)
    {
        code->localized_leave--;
    } /* the localized_leave = 1 case is where the interpreter was left initially */
    if (code->localized_leave)
    {
        return 1;
    }

    if (code->location == 0)
    {
        interpret_start(code);
        if (start != 0L)
        {
            (*start)(code, structure, identifier);
        }
    }

    if (interpret_code(code) == -1)
    {
        return -1;
    }

    if (code->location == 0)
    {
        if ((code != 0L) && (end != 0L))
        {
            (*end)(code, structure, identifier);
        }
    }

    code->localized_leave = code->leave;

    if (exit_offset > -1)
    {
        n_int * variables = code->variable_references;
        if (variables[exit_offset] == 0)
        {
            return 1;
        }
    }
    return 0;
}


