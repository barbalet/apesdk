/****************************************************************

 commands.h

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


/*NOBLEMAKE VAR=""*/

#ifndef _NOBLEAPE_COMMANDS_H_
#define _NOBLEAPE_COMMANDS_H_

/*
    This is intended to be overwritten by third-party usage where applicable.
 */

enum SECONDARY_APESCRIPT
{
    VARIABLE_RANDOM = (VARIABLE_IF + 1),
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

static variable_string	variable_codes[VARIABLE_MAX]=
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

n_int commands_output(void * vcode, n_byte * kind, n_int * number);

n_int commands_input(void *code, n_byte kind, n_int value);


#endif

