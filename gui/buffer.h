/****************************************************************

 buffer.h

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

#ifndef SIMULATEDAPE_BUFFER_H
#define SIMULATEDAPE_BUFFER_H

#ifndef _WIN32

#include "../toolkit/toolkit.h"

#else

#include "..\toolkit\toolkit.h"

#endif

typedef struct
{
    n_byte *current;
    n_byte *previous;
    n_int   maximum_bytes;
} ape_buffer;

n_byte *ape_buffer_require( ape_buffer *buffer, n_int width, n_int height );
void ape_buffer_free( ape_buffer *buffer );
n_byte *ape_buffer_rgba_to_rgb24( n_byte *copy_in, n_uint size );

#endif /* SIMULATEDAPE_BUFFER_H */
