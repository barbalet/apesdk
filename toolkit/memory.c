/****************************************************************

 memory.c

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

/*! \file   memory.c
 *  \brief  Covers the low level input and output relating to memory.
 */

#include "toolkit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static memory_execute * static_execution = 0;

void memory_execute_set(memory_execute * value)
{
    static_execution = value;
}

void memory_execute_run(void)
{
    if (static_execution)
    {
        static_execution();
    }
}

/**
 * This is a historical legacy function as all platforms now use memcpy. Although in the future this may change.
 * @param from pointer to copy from.
 * @param to pointer to copy to.
 * @param number the number of bytes to copy.
 */
void memory_copy( n_byte *from, n_byte *to, n_uint number )
{
    memcpy( to, from, number );
}

/**
 * This is a historical legacy function as all platforms now use malloc. Although in the future this may change.
 * @param bytes number of bytes to allocate.
 * @return a void* pointer of the allocated bytes.
 */
void 	*memory_new( n_uint bytes )
{
    void 	*tmp = 0L;
    if ( bytes )
    {
        tmp = ( void * ) malloc( bytes );
    }
    return ( tmp );
}

/**
 * This is a historical legacy function as all platforms now use free. Although in the future this may change.
 * @param ptr the void * pointer to be freed. Should really be a void ** to catch the 0L-ing.
 */
void memory_free( void **ptr )
{
    if ( *ptr != 0L )
    {
        free( *ptr );
        *ptr = 0L;
    }
}

/**
 * This is allocates a range of memory depending on availability.
 * @param memory_min the minimum possible allocated memory before returning 0L.
 * @param memory_allocated the starting value for memory size and returning the actual size.
 * @return a void* pointer of the allocated bytes.
 */
void *memory_new_range( n_uint memory_min, n_uint *memory_allocated )
{
    void *memory_buffer = 0L;
    do
    {
        memory_buffer = ( void * ) malloc( *memory_allocated );
        if ( memory_buffer == 0L )
        {
            *memory_allocated = ( ( *memory_allocated ) * 3 ) >> 2;
        }
    }
    while ( ( memory_buffer == 0L ) && ( ( *memory_allocated ) > memory_min ) );
    return memory_buffer;
}


void memory_erase( n_byte *buf_offscr, n_uint nestop )
{
    memset( buf_offscr, 0, nestop );
}

memory_list *memory_list_new( n_uint size, n_uint number )
{
    memory_list *new_list = ( memory_list * )memory_new( sizeof( memory_list ) );
    if ( new_list )
    {
        new_list->data = ( n_byte * )memory_new( size * number );
        if ( new_list->data == 0L )
        {
            memory_free( ( void ** )&new_list );
            return 0L;
        }
        new_list->count = 0;
        new_list->max = number;
        new_list->unit_size = size;
    }
    return new_list;
}

void memory_list_copy( memory_list *list, n_byte *data, n_uint size)
{
    memory_copy( data, &( list->data[list->unit_size * list->count] ), size );
    list->count += (size /(list->unit_size));
    if (size % (list->unit_size))
    {
        (void)SHOW_ERROR("wrong base unit size");
    }

    if ( list->count >= list->max )
    {
        n_uint   new_max = ( list->max * 2 );
        n_uint   new_size = new_max * list->unit_size;
        n_byte *new_range = memory_new( new_size );
        NA_ASSERT( new_range, "range failed to allocate" );
        if ( new_range )
        {
            memory_copy( list->data, new_range, new_size / 2 );
            memory_free( ( void ** ) & ( list->data ) );
            list->max = new_max;
            list->data = new_range;
        }
    }
}

void memory_list_free( memory_list **value )
{
    memory_list *list = *value;
    memory_free( ( void ** ) & ( list->data ) );
    memory_free( ( void ** )value );
}

int_list *int_list_new( n_uint number )
{
    return (int_list *)memory_list_new( sizeof(n_int), number );
}

void int_list_copy( int_list *list, n_int int_add)
{
    memory_list_copy(list, (n_byte *) &int_add, sizeof(int_add));
}

void int_list_free( int_list **value )
{
    memory_list_free(value);
}

n_int int_list_find( int_list * list,  n_int location, n_int * error)
{
    n_int * data_int = (n_int *) list->data;

    if ((location > list->count) || (location < 0))
    {
        *error = -1;
        return SHOW_ERROR("Out of Bounds failure");
    }
    *error = 0;
    return data_int[location];
}

void int_list_debug( int_list * debug_list)
{
    n_int count = debug_list->count, loop = 0, error = 0;
    printf("count %ld max %ld\n - - - - - - - - - - - - - - - - - - -\n", debug_list->count, debug_list->max);
    while (loop < count)
    {
        n_int value = int_list_find( debug_list,  loop, &error );
        if (error == 0)
        {
            if (value == BIG_INTEGER)
            {
                printf("| ");
            }
            else
            {
                if ((value - BIG_NEGATIVE_INTEGER) <= object_get_hash_count())
                {
                    n_uint key_value = value - BIG_NEGATIVE_INTEGER;
                    printf("\n(%ld), ", key_value);
                }
                else
                {
                    printf("%ld, ", value);
                }
            }
        }
        else
        {
            printf("%ld (%ld), ", value, error);
        }
        loop++;
    }
    printf("\n");
}

number_array_list * number_array_list_new(void)
{
    return (number_array_list *) memory_list_new(sizeof(number_array), 10);
}

void number_array_list_free(number_array_list ** nal)
{
    memory_list_free( (memory_list**) nal );
}

static void number_array_list_copy(number_array_list * nal, number_array * na)
{
    memory_list_copy(nal, (n_byte *)na, sizeof(number_array));
}

number_array * number_array_list_find(number_array_list * nal, void * array)
{
    n_int loop = 0;
    number_array * checkptr = (number_array *)nal->data;
    while (loop < nal->count)
    {
        if (checkptr[loop].array == array)
        {
            return &checkptr[loop];
        }
        loop++;
    }
    return 0L;
}

number_array * number_array_list_find_add(number_array_list * nal, void * array)
{
    n_int loop = 0;
    number_array * checkptr = (number_array *)nal->data;
    while (loop < nal->count)
    {
        if (checkptr[loop].array == array)
        {
            return &checkptr[loop];
        }
        loop++;
    }

    number_array * return_value = memory_new(sizeof(number_array));
    return_value->array = array;
    return_value->number = int_list_new(8);

    number_array_list_copy(nal, return_value);

    return return_value;
}

void number_array_not_number(number_array * na)
{
    if (na)
    {
        if (na->number)
        {
            int_list_free(&(na->number));
        }
        (void)SHOW_ERROR("number array values not present (clear)");
    }
    (void)SHOW_ERROR("number array not present (clear)");
}

void number_array_number(number_array * na, n_int number)
{
    if (na)
    {
        if (na->number)
        {
            int_list_copy(na->number, number);
        }
        else
        {
            (void)SHOW_ERROR("number array values not present (copy)");
            return;
        }
    }
    else
    {
        (void)SHOW_ERROR("number array not present (copy)");
    }
}

n_int number_array_get_number(number_array * na, n_int location, n_int * error)
{
    if (na)
    {
        if (na->number)
        {
            return int_list_find(na->number, location, error);
        }
        * error = -3;
        return SHOW_ERROR("number array values not present");
    }
    * error = -2;
    return SHOW_ERROR("number array not present");
}

n_int number_array_get_size(number_array * na)
{
    if (na)
    {
        if (na->number)
        {
            return na->number->count;
        }
        return SHOW_ERROR("number array values not present (size)");
    }
    return SHOW_ERROR("number array not present (size)");
}
