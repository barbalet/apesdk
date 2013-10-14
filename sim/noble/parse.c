/****************************************************************

 parse.c

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

/*! \file   parse.c
 *  \brief  This parses ApeScript and produces the ApeScript byte-code.
 */

/*NOBLEMAKE DEL=""*/
#include "noble.h"
/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/

#if defined(ROUGH_CODE_OUT) || defined(COMMAND_LINE_DEBUG)

#include "stdio.h"

#endif


#define	SYNTAX_NUM				19
#define	SYNTAX_WIDTH			4
static const n_byte	syntax_codes[SYNTAX_NUM][SYNTAX_WIDTH]=
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
/*NOBLEMAKE END=""*/
#ifdef SCRIPT_DEBUG
/*NOBLEMAKE DEL=""*/
/*NOBLEMAKE END=""*/

static n_int	          tab_step = 0;
static variable_string	* local_var_codes;

n_file                  * file_debug = 0L;
static n_int              single_entry = 1;

n_file * scdebug_file_ready(void)
{
    if (single_entry)
    {
        return 0L;
    }
    return file_debug;
}

void scdebug_file_cleanup(void)
{
    /* This setting to zero may be duplicated in at least one place
     but provides additional protection - it may not be needed following
     a case-by-case review */
    
    single_entry = 0;
    
    if (file_debug)
    {
        io_file_free(file_debug);
    }
    file_debug = 0L;
}

void scdebug_writeon(void)
{
    if (single_entry == 0) return;
#ifndef COMMAND_LINE_DEBUG
    if (file_debug == 0L) /* io_file_reused */
    {
        file_debug = io_file_new();
    }

    if(file_debug == 0L)
    {
        return;
    }

    if(file_debug->data == 0L)
    {
        scdebug_file_cleanup();
        return;
    }
    single_entry = 1;
#endif
}

void scdebug_writeoff(void)
{
    if (single_entry == 0) return;
#ifndef COMMAND_LINE_DEBUG
    if(file_debug != 0L)
    {
        single_entry = 0;
    }
#endif
}

n_string scdebug_variable(n_int variable)
{
    n_string return_value = 0L;
    if((variable < VARIABLE_MAX)
#ifndef COMMAND_LINE_DEBUG
            && (file_debug  != 0L)
#endif
      )
    {
        return_value = (n_string) local_var_codes[variable];
    }
    return return_value;
}

void scdebug_string(n_constant_string string)
{
    if (single_entry == 0) return;

    if((string != 0L)
#ifndef COMMAND_LINE_DEBUG
            && (file_debug  != 0L)
#endif
      )
    {
#ifndef COMMAND_LINE_DEBUG
        io_write(file_debug, string, 0);
#else
        printf("%s",string);
#endif
    }
}

void scdebug_int(n_int number)
{
    if (single_entry == 0) return;

#ifndef COMMAND_LINE_DEBUG
    if(file_debug  != 0L)
    {
        io_writenumber(file_debug, number, 1, 0);
    }
#else
    printf("%d",(int)number);
#endif
}

void scdebug_newline(void)
{
    if (single_entry == 0) return;

#ifndef COMMAND_LINE_DEBUG
    if(file_debug != 0L)
#endif
    {
        n_int loop = 0;
#ifndef COMMAND_LINE_DEBUG
        io_write(file_debug, "", 1);
#else
        printf("\n");
#endif
        if(tab_step > 0)
        {
            while(loop<tab_step)
            {
#ifndef COMMAND_LINE_DEBUG
                io_write(file_debug,"  ",0);
#else
                printf("  ");
#endif
                loop++;
            }
        }
    }
}

void scdebug_tabstep(n_int steps)
{
#ifndef COMMAND_LINE_DEBUG
    if(file_debug != 0L)
#endif
    {
        tab_step += steps;
    }
}
#endif

/* outputs the number of bytes to advance in the interpret stream */
static n_int parse_number(n_interpret * interpret, const n_byte * number)
{
    n_int 	loop = 0;
    n_int 	out_value = 0;
    n_int 	point_counter = 0;

    /* read out the number from the interpret stream */
    do
    {
        n_byte temp = number[point_counter++];
        if((!ASCII_NUMBER(temp)) && (temp != 0))
        {
            return io_apescript_error(AE_NUMBER_EXPECTED); /* this error should never occur */
        }
        out_value = (out_value * 10) + (temp - '0');
    }
    while((number[point_counter]!=0) && (out_value>-1));

    if((out_value < 0) || (out_value > 0x7fffffff))
    {
        return io_apescript_error(AE_NUMBER_OUT_OF_RANGE);
    }

    /* is this number already stored? */
    while(loop < number_num)
    {
        if(interpret->number_buffer[loop] == out_value)
            return loop;
        loop++;
    }
    /* if not, add it to the number store */
    interpret->number_buffer[loop] = out_value;
    if(number_num < NUMBER_MAX)
    {
        number_num++;
    }
    else
    {
        return io_apescript_error(AE_MAXIMUM_NUMBERS_REACHED);
    }
    return loop;
}

static n_byte parse_character(n_byte temp)
{
    if(ASCII_BRACES(temp) || ASCII_BRACKET(temp))
        return temp;
    if((ASCII_EQUAL(temp) || ASCII_LOGICAL(temp))||(ASCII_ARITHMETIC(temp) || ASCII_DIRECTIONAL(temp)))
        return '=';
    if(ASCII_NUMBER(temp))
        return 'n';
    if(ASCII_TEXT(temp))
        return 't';
    if(ASCII_SEMICOLON(temp))
        return ';';
    return 'F';
}

static n_int parse_write_code(n_interpret * final_prog, n_byte value, n_byte code)
{
#ifdef ROUGH_CODE_OUT
    FILE * rough_code_file = fopen("rough_code.txt","a");
    fprintf(rough_code_file, "%c ",value);
#endif

    if(io_file_write(final_prog->binary_code, value) == -1)
    {
        return io_apescript_error(AE_MAXIMUM_SCRIPT_SIZE_REACHED);
    }
    if(CODE_VALUE_REQUIRED(value))
    {
        if(io_file_write(final_prog->binary_code, code) == -1)
        {
            return io_apescript_error(AE_MAXIMUM_SCRIPT_SIZE_REACHED);
        }

#ifdef ROUGH_CODE_OUT
        fprintf(rough_code_file, "%d ",code);
#endif
    }

#ifdef ROUGH_CODE_OUT
    if (value == ';' || value == '{' || value == '}')
    {
        fprintf(rough_code_file, "\n");
    }
    if (fclose(rough_code_file) != 0)
    {
        return SHOW_ERROR("File failed to close");
    }
#endif
    return 0;
}

static n_int parse_string(const n_byte * test, const n_byte * compare, n_int number)
{
    n_int		loop = 0;
    while(loop<number)
    {
        if(test[loop] != compare[loop])
            return -1;
        loop++;
    }
    return 1;
}

static n_int parse_buffer(n_interpret * final_prog, n_byte previous, const n_byte * buffer)
{
    variable_string *variable_codes = final_prog->variable_strings;
    n_int			 result = -1;
    n_int			 loop = 0;
    switch(previous)
    {
    case ('n'):
        result = parse_number(final_prog, buffer); /* this loads the number into the number buffer */
        if(result == -1)
        {
            return -1;
        }
        if(parse_write_code(final_prog, previous, (n_byte)result) == -1)  /* this writes the number allocation code */
        {
            return -1;
        }
        break;
    case ('t'):
        while((loop < variable_num) && (result == -1))
        {
            if(parse_string(variable_codes[loop], buffer, VARIABLE_WIDTH) == 1)
            {
                result = loop;
            }
            loop++;
        }
        if(result == -1)
        {
            if(variable_num < VARIABLE_MAX)
            {
                n_int loop2 = 0;
                while(loop2 < (VARIABLE_WIDTH))
                {
                    variable_codes[variable_num][loop2] = buffer[loop2];
                    loop2++;
                }
                variable_num++;
            }
            else
            {
                return io_apescript_error(AE_MAXIMUM_VARIABLES_REACHED);
            }
            result = loop;
        }
        if(parse_write_code(final_prog, previous, (n_byte)result) == -1)
        {
            return -1;
        }
        break;
    case ('='):
        while((loop < SYNTAX_NUM) && (result == -1))
        {
            if(parse_string(syntax_codes[loop],buffer,SYNTAX_WIDTH) == 1)
            {
                result = loop;
            }
            loop++;
        }
        if(result == -1)  /* no error reported up until now */
        {
            return io_apescript_error(AE_UNKNOWN_SYNTAX_PARSER_BUFFER);
        }
        if(parse_write_code(final_prog, previous, (n_byte)result) == -1)
        {
            return -1;
        }
        break;
    default:
    {
        n_byte	value;
        while((value = buffer[loop++]) != 0)
        {
            if(parse_write_code(final_prog, value, 0) == -1)
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
n_interpret *	parse_convert(n_file * input, n_int main_entry, variable_string * variables)
{
    n_interpret * final_prog = 0L;
    n_byte	    * local_data;
    n_uint	      end_loop;
    n_uint	      loop = 0;
    n_int	    * local_number;
    n_byte	      buffer[ VARIABLE_WIDTH ];
    n_int	      buffer_size = 0;
    n_byte	      previous = 0;

    io_whitespace(input);

    if((final_prog = io_new(sizeof(n_interpret))) == 0L)
    {
        return 0L;
    }

    final_prog->variable_references = 0L;

    if((final_prog->binary_code = io_file_new())== 0L)
    {
        io_free(final_prog);
        return 0L;
    }

    if(final_prog->binary_code->data == 0L)
    {
        interpret_cleanup(final_prog);
        return 0L;
    }

    final_prog->binary_code->location = SIZEOF_NUMBER_WRITE;

    if((final_prog->variable_references = (n_int *)io_new(VARIABLE_MAX * sizeof(n_int))) == 0L)
    {
        interpret_cleanup(final_prog);
        return 0L;
    }

    final_prog->variable_strings = variables;
    final_prog->special_less    = (VARIABLE_IF + 1);
    final_prog->main_entry      = main_entry;

    local_number = final_prog->number_buffer;

    variable_num = main_entry + 1;

    number_num = 1;

    while(loop < NUMBER_MAX)
    {
        local_number[ loop++ ] = 0;
    }
    loop = 0;
    io_erase(buffer, VARIABLE_WIDTH);

    local_data = input->data;
    end_loop = input->size;

    while(loop < end_loop)
    {
        n_byte	temp = local_data[ loop++ ];
        n_byte	convert = parse_character(temp);
        if(convert == 'F')
        {
            interpret_cleanup(final_prog);
            (void)io_apescript_error(AE_UNKNOWN_SYNTAX_PARSER_CONVERT);
            return 0L;
        }
        if((previous != convert) && (previous != 0))
        {
            if(parse_buffer(final_prog, previous, buffer) == -1)
            {
                interpret_cleanup(final_prog);
                return 0L;
            }
            buffer_size = 0;
            io_erase(buffer, VARIABLE_WIDTH);
        }
        buffer[buffer_size++] = temp;
        if(buffer_size == (VARIABLE_WIDTH -  1))
        {
            interpret_cleanup(final_prog);
            (void)io_apescript_error(AE_MAXIMUM_SCRIPT_SIZE_REACHED);
            return 0L;
        }
        previous = convert;
    }
    if(parse_buffer(final_prog, previous, buffer) == -1)
    {
        interpret_cleanup(final_prog);
        return 0L;
    }
    {
        n_byte	local_numbers[SIZEOF_NUMBER_WRITE];
        n_uint   loop_sizeof_number;
        /* this is the one special case for direct writing as the original stamp size was allowed */
        io_int_to_bytes(final_prog->binary_code->location,final_prog->binary_code->data); /* write the basic size header */
        end_loop = number_num;
        loop = 1;
        io_int_to_bytes(number_num,local_numbers); /* write the number of numbers */
        loop_sizeof_number = 0;
        while (loop_sizeof_number < SIZEOF_NUMBER_WRITE)
        {
            if(io_file_write(final_prog->binary_code, local_numbers[loop_sizeof_number]) == -1)
            {
                interpret_cleanup(final_prog);
                return 0L;
            }
            loop_sizeof_number++;
        }
        while(loop<end_loop)
        {
            io_int_to_bytes((final_prog->number_buffer[loop]),local_numbers); /* write the numbers */
            loop_sizeof_number = 0;
            while (loop_sizeof_number < SIZEOF_NUMBER_WRITE)
            {
                if(io_file_write(final_prog->binary_code, local_numbers[loop_sizeof_number]) == -1)
                {
                    interpret_cleanup(final_prog);
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

