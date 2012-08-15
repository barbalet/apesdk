/****************************************************************

 skim.c

 =============================================================

 Copyright 1996-2012 Tom Barbalet. All rights reserved.

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


/*! \file   skim.c
 *  \brief  This code is maintained to translate between BrainCode
    and ApeScript bytecode. This code has the eventual aim of providing
    a uniform 1:1 interface and allow dynamic ApeScript to evolve and
    produce a human readable version of BrainCode. Could be early days yet,
    time will tell.
 */

/*NOBLEMAKE DEL=""*/
#include "noble.h"
/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/


#ifdef SKIM_TO_BRIANCODE

#include "stdio.h"

static void skim_clear_pipe(n_byte *pipe)
{
    pipe[0]=0;
    pipe[1]=0;
    pipe[2]=0;
    pipe[3]=0;
    pipe[4]=0;
    pipe[5]=0;
    pipe[6]=0;
    pipe[7]=0;
    pipe[8]=0;
    pipe[9]=0;
    pipe[10]=0;
    pipe[11]=0;
}


static n_string skim_arithmetic(n_byte value)
{
    switch (value)
    {
    case SYNTAX_MINUS:
        return "SUB";
    case SYNTAX_ADDITION:
        return "ADD";

    case SYNTAX_MULTIPLY:
        return "MUL";

    case SYNTAX_AND:
        return "AND";

    case SYNTAX_XOR:
        return "XOR";

    case SYNTAX_OR:
        return "OR ";

    case SYNTAX_GREATER_THAN:
        return "GTH";

    case SYNTAX_LESS_THAN:
        return "LTH";

    case SYNTAX_EQUAL_TO:
        return "EQT";

    case SYNTAX_NOT_EQUAL_TO:
        return "NET";

    case SYNTAX_LESS_EQUAL:
        return "LTE";

    case SYNTAX_GREATER_EQUAL:
        return "GTE";

    case SYNTAX_CONDITIONAL_AND:
        return "CAN";

    case SYNTAX_CONDITIONAL_OR:
        return "COR";

    case SYNTAX_DIVISION:
        return "DIV";

    case SYNTAX_MODULUS:
        return "MOD";

    case SYNTAX_BITSHIFT_RIGHT:
        return "BSR";

    case SYNTAX_BITSHIFT_LEFT:
        return "BSL";

    case SYNTAX_MOVE:
        return "MOV";

    case SYNTAX_JUMP_TO:
        return "JMP";

    case SYNTAX_JUMP_EQUAL_ZERO:
        return "JMZ";

    case SYNTAX_DATA:
        return "DAT";

    default:
        return "Unkown value";
    }
}

#define CONSTANT_VARIABLE_COMPARISON(value,num) \
            if (pipe[num] == 't') \
                value = pipe[num+1] + 128; \
            else \
                value = pipe[num+1];

static n_byte parse_handle_pipe(n_byte line_number, n_byte * pipe, n_int count)
{
    n_byte  add_line_number = 0;
    n_byte  first_value;
    n_byte  second_value;

    printf("                               %d:\n", line_number);

    if (line_number == 0)
    {
        /* this is currently unknown - working on a method to know this by this stage */
        printf("                                   JMP %d ---\n", -1);
        add_line_number += 2;
    }

    if (count == 7)
    {
        if (pipe[1] == VARIABLE_FUNCTION)
        {
            printf("                                   DAT %d %d\n", pipe[1], pipe[4]+128);
            add_line_number += 3;
        }
        if (pipe[3] == SYNTAX_EQUALS)
        {
            CONSTANT_VARIABLE_COMPARISON(second_value, 4);
            printf("                                   MOV %d %d\n", pipe[1], second_value);
            add_line_number += 3;
        }
        if (pipe[1] == VARIABLE_RUN)
        {
            printf("                                   JMP %d ---\n", pipe[4]+128);
            add_line_number += 2;
        }
    }
    else if (count == 11)
    {
        if (pipe[3] == SYNTAX_EQUALS)
        {
            n_string arithmetic = skim_arithmetic(pipe[7]);
            CONSTANT_VARIABLE_COMPARISON(first_value, 4);
            CONSTANT_VARIABLE_COMPARISON(second_value, 9);

            printf("                                   %s %d %d\n", arithmetic, first_value, second_value);
            printf("                                   MOV %d %d\n", pipe[1], 255);
            add_line_number += 6;
        }
        else if (pipe[1] == VARIABLE_IF)
        {
            n_string arithmetic = skim_arithmetic(pipe[6]);
            CONSTANT_VARIABLE_COMPARISON(first_value, 3);
            CONSTANT_VARIABLE_COMPARISON(second_value, 7);

            printf("                                   %s %d %d\n", arithmetic, first_value, second_value);
            /* This is unknown */
            printf("                                   JMZ %d %d\n", 255, -1);
            add_line_number += 6;
        }
    }
#ifdef LINE_NUMBER_NEEDED
    printf("                                             %d\n",(int)count);
#endif
    return add_line_number;
}

#define SHOW_INITIAL_CODE

void skim_show(n_file * binary_code)
{
    n_byte   pipe[12] = {0};
    n_int    count = 0;
    n_byte   line_count = 0;
    n_byte * local_buffer = &((binary_code->data)[SIZEOF_NUMBER_WRITE]);
    n_int    loop = 0;
    n_int    loop_end = BYTES_TO_INT(binary_code->data) - SIZEOF_NUMBER_WRITE;
    while (loop < loop_end)
    {
        n_byte value = local_buffer[loop++];
        pipe[count] = value;
        count++;
#ifdef SHOW_INITIAL_CODE
        printf(" %c ",value);
#endif
        if (CODE_VALUE_REQUIRED(value))
        {
            n_byte code = local_buffer[loop++];
            pipe[count] = code;
#ifdef SHOW_INITIAL_CODE
            printf("%d ",code);
#endif
            count++;
        }

        if (value == ';' || value == '{' || value == '}')
        {
#ifdef SHOW_INITIAL_CODE
            printf("\n");
#endif
            line_count += parse_handle_pipe(line_count, pipe, count);
            skim_clear_pipe(pipe);
            count = 0;
        }
    }
}

#endif
