/****************************************************************

 memory.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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


/**
 * This is a historical legacy function as all platforms now use memcpy. Although in the future this may change.
 * @param from pointer to copy from.
 * @param to pointer to copy to.
 * @param number the number of bytes to copy.
 */
void memory_copy(n_byte * from, n_byte * to, n_uint number)
{
    memcpy(to, from, number);
}

/**
 * This is a historical legacy function as all platforms now use malloc. Although in the future this may change.
 * @param bytes number of bytes to allocate.
 * @return a void* pointer of the allocated bytes.
 */
void *	memory_new(n_uint bytes)
{
    void *	tmp = 0L;
    if (bytes)
    {
        tmp = (void *) malloc(bytes);
    }
    return (tmp);
}

/**
 * This is a historical legacy function as all platforms now use free. Although in the future this may change.
 * @param ptr the void * pointer to be freed. Should really be a void ** to catch the 0L-ing.
 */
void memory_free(void ** ptr)
{
    if (*ptr != 0L)
    {
        free(*ptr);
        *ptr = 0L;
    }
}

/**
 * This is allocates a range of memory depending on availability.
 * @param memory_min the minimum possible allocated memory before returning 0L.
 * @param memory_allocated the starting value for memory size and returning the actual size.
 * @return a void* pointer of the allocated bytes.
 */
void * memory_new_range(n_uint memory_min, n_uint *memory_allocated)
{
    void * memory_buffer = 0L;
    do
    {
        memory_buffer = (void *) malloc(*memory_allocated);
        if (memory_buffer == 0L)
        {
            *memory_allocated = ((*memory_allocated) * 3) >> 2;
        }
    }
    while((memory_buffer == 0L) && ((*memory_allocated) > memory_min));
    return memory_buffer;
}


void memory_erase(n_byte * buf_offscr, n_uint nestop)
{
    memset(buf_offscr, 0, nestop);
}
