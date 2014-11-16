/****************************************************************

 message.c

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

#include "gui.h"

typedef struct
{
    void * next;
    n_string message;
    n_int * spaces;
    n_string_block * direct_rendering;
    n_int x;
    n_int y;
    n_int width;
    n_int height;
    n_int time_to_expire;
} n_message;

static n_message * first_message = 0L;

static n_message * message_find_last(void)
{
    n_message * return_message = first_message;
    if (return_message == 0L)
    {
        return 0L;
    }
    if (return_message->next)
    {
        do
        {
            return_message = return_message->next;
        }while(return_message->next);
    }
    return return_message;
}

void message_add(n_string message, n_int time_to_expire)
{
    n_message * return_message = io_new(sizeof(n_message));
    if (return_message == 0L)
    {
        return;
    }
    return_message->next = 0L;
    return_message->message = message;
    return_message->time_to_expire = time_to_expire;

    if (first_message == 0L)
    {
        first_message = return_message;
    }
    else
    {
        n_message * previous_message = message_find_last();
        previous_message->next = return_message;
    }
}

