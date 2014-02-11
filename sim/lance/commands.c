/****************************************************************

 commands.c

 =============================================================

 Copyright 1996-2014 Tom Barbalet. All rights reserved.

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



#ifndef	_WIN32

#include "../noble/noble.h"

#else

#include "..\noble\noble.h"

#endif

#include "commands.h"

#include <unistd.h>
#include <stdio.h>
#include <time.h>

static n_byte2  random_seed[2];
static n_byte   random_populated = 0;
static n_int	persist_value = 0;
static n_uint   random_noise = 0x6d73e3a3;

static void commands_random_seed(n_int seed_value)
{
    random_populated = 1;

    if (seed_value < 0)
    {
        n_uint unsignedvalue = time(NULL);
        switch (random_noise % 2)
        {
        case 0:
            random_noise   ^= (unsignedvalue ^ 0x47c23e19);
            break;
        case 1:
            random_noise   ^= (unsignedvalue ^ 0x9ef1de93);
            break;
        case 2:
            random_noise   ^= (unsignedvalue ^ 0x2e97c735);
            break;
        }
        random_seed[0] ^= (random_noise>>16) & 0xFFFF;
        random_seed[1] ^=  random_noise & 0xFFFF;
    }
    else
    {
        n_uint  seed = ABS(seed_value);

        seed = (seed * 2999)&0xFFFFFFFF;

        random_seed[0] = (seed>>16) & 0xFFFF;
        random_seed[1] =  seed & 0xFFFF;
    }
}

static n_byte2 commands_handle_random()
{
    if (random_populated == 0)
    {
        commands_random_seed(-1);
    }
    return math_random(random_seed);
}

/* notification here is optional and it can be done asynchronously */

static void commands_notify_done(n_int * notification)
{
    notification[0] = 0;
}

static void commands_process_run(n_int value, n_int * notification)
{
    /* do not create any additional noise if the values aren't found */
    switch (value)
    {
    case FUNCTION_ONE_TRIGGER_NUMBER:
    case FUNCTION_TWO_TRIGGER_NUMBER:
    case FUNCTION_THREE_TRIGGER_NUMBER:
    case FUNCTION_FOUR_TRIGGER_NUMBER:
    case FUNCTION_FIVE_TRIGGER_NUMBER:
    case FUNCTION_SIX_TRIGGER_NUMBER:
        /* best not to add a new line for the debug format */
        commands_notify_done(notification);
        break;
    }
}

n_int commands_input(void *vindividual, n_byte kind, n_int value)
{
    n_individual_interpret *individual = (n_individual_interpret *)vindividual;
    n_int *local_vr = individual->variable_references;

    if (kind > VARIABLE_READWRITE)
    {

        if (kind == VARIABLE_PERSIST)
        {
            persist_value = value;
        }
        else
        {
            local_vr[kind - VARIABLE_FIRST_REAL_ONE] = value;
        }


        if (kind == VARIABLE_RUN_EXTERNAL_FUNCTION)
        {
            commands_process_run(value, &local_vr[kind - VARIABLE_FIRST_REAL_ONE]);
        }

        if (kind == VARIABLE_RANDOM_SEED)
        {
            commands_random_seed(value);
        }
        if (kind == VARIABLE_ESCAPE)
        {
            individual->leave = value;
        }
        return 0;
    }
    return -1; /* where this fails is more important than this failure */
}

n_int commands_output(void * vcode, void * vindividual, n_byte * kind, n_int * number)
{
    n_interpret *code = (n_interpret *)vcode;
    n_individual_interpret *individual = (n_individual_interpret *)vindividual;
    n_byte	first_value = kind[0];
    n_byte	second_value = kind[1];
    if(first_value == 'n')
    {
        *number = code->number_buffer[second_value];
        return 0;
    }
    if((first_value == 't') && (VARIABLE_SPECIAL(second_value, code)==0))
    {
        n_int	*local_vr = individual->variable_references;

        if(second_value>VARIABLE_READWRITE)
        {
            if (second_value == VARIABLE_PERSIST)
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
            switch(second_value)
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
