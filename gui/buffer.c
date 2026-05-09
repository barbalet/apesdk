/****************************************************************

 buffer.c

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

#include "buffer.h"

n_byte *ape_buffer_require( ape_buffer *buffer, n_int width, n_int height )
{
    n_int required_bytes;

    if ( buffer == 0L )
    {
        return 0L;
    }

    required_bytes = width * height * 4;

    if ( buffer->previous )
    {
        memory_free( ( void ** )&buffer->previous );
    }

    if ( ( buffer->current == 0L ) || ( required_bytes > buffer->maximum_bytes ) )
    {
        buffer->maximum_bytes = required_bytes;
        buffer->previous = buffer->current;
        buffer->current = memory_new( required_bytes );
    }

    return buffer->current;
}

void ape_buffer_free( ape_buffer *buffer )
{
    if ( buffer == 0L )
    {
        return;
    }

    if ( buffer->previous )
    {
        memory_free( ( void ** )&buffer->previous );
    }

    if ( buffer->current )
    {
        memory_free( ( void ** )&buffer->current );
    }

    buffer->maximum_bytes = 0;
}

n_byte *ape_buffer_rgba_to_rgb24( n_byte *copy_in, n_uint size )
{
    n_byte *return_value = memory_new( size * 3 );
    n_uint  loop = 0;
    n_uint  loop3 = 0;
    n_uint  loop4 = 0;

    if ( return_value == 0L )
    {
        return 0L;
    }

    while ( loop < size )
    {
        loop4++;
        return_value[loop3++] = copy_in[loop4++];
        return_value[loop3++] = copy_in[loop4++];
        return_value[loop3++] = copy_in[loop4++];
        loop++;
    }

    return return_value;
}
