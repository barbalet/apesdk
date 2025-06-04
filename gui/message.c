/****************************************************************

 message.c

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

#include "gui.h"

typedef struct
{
    n_string message;
    n_int *spaces;
    n_string_block *direct_rendering;
    n_int x;
    n_int y;
    n_int width;
    n_int height;
    n_int time_to_expire;
} n_message;

#define MAXIMUM_NUMBER_MESSAGE 10

static n_int number_messages = 0;

n_message messages[MAXIMUM_NUMBER_MESSAGE];

static n_int *message_find_spaces( n_string string, n_int *count )
{
    n_int local_count = 0;
    n_int loop = 0;
    n_int *return_value = 0L;

    if ( *count )
    {
        return_value = memory_new( ( n_uint )( *count ) * sizeof( n_int ) );
    }

    if ( string == 0L )
    {
        return 0L;
    }

    if ( string[0] == 0 )
    {
        return 0L;
    }

    do
    {
        if ( string[loop] == ' ' )
        {
            if ( *count )
            {
                return_value[local_count] = loop;
            }
            local_count++;

        }
    }
    while ( string[loop] );

    return return_value;
}

static void message_remove( n_int remove )
{
    n_message *value = &messages[remove];

    if ( value )
    {
        if ( value->message )
        {
            memory_free( ( void ** ) & ( value->message ) );
        }
        if ( value->spaces )
        {
            memory_free( ( void ** ) & ( value->spaces ) );
        }
    }
    if ( remove != ( number_messages - 1 ) )
    {
        n_message *copy_from = &messages[number_messages - 1];
        value->message = copy_from->message;
        value->spaces = copy_from->spaces;
        value->direct_rendering = copy_from->direct_rendering;
        value->x = copy_from->x;
        value->y = copy_from->y;
        value->width = copy_from->width;
        value->height = copy_from->height;
        value->time_to_expire = copy_from->time_to_expire;
    }
    number_messages--;
}

#if 0

static void message_add( n_string message, n_int time_to_expire )
{
    n_int   *spaces;
    n_string copied_message;
    n_int    space_count = 0;

    if ( number_messages == ( MAXIMUM_NUMBER_MESSAGE - 1 ) )
    {
        /* if the message stack is full remove the oldest message */
        n_int loop = 0;
        n_int oldest_loop = 0;
        n_int oldest_time_to_expire = 100000;
        while ( loop < number_messages )
        {
            if ( oldest_time_to_expire > messages[loop].time_to_expire )
            {
                oldest_loop = loop;
                oldest_time_to_expire = messages[loop].time_to_expire;
            }
            loop++;
        }
        message_remove( oldest_loop );
    }

    if ( message == 0L )
    {
        return;
    }
    if ( time_to_expire == 0 )
    {
        return;
    }
    ( void )message_find_spaces( message, &space_count );
    if ( space_count == 0 )
    {
        return;
    }
    spaces = message_find_spaces( message, &space_count );
    if ( spaces == 0L )
    {
        return;
    }
    copied_message = io_string_copy( message );
    if ( copied_message == 0L )
    {
        memory_free( ( void ** ) & ( spaces ) );
        return;
    }
    messages[number_messages].time_to_expire = time_to_expire;
    messages[number_messages].message = copied_message;
    messages[number_messages].spaces = spaces;
    number_messages++;
}

#endif


